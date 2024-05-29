#include "head.h"

int Download(int sockFd, int user_id)//puts
{
    // 先获取文件名，不同客户端传的文件可能文件名相同但文件内容不同，所以根据md5值来判断本地是否有该文件
    char name[512] = {0};
    int dataLength;

    //我第一次接收客户端发来的消息，有两种情况，文件打开失败或者是文件名

    train_t t;//接收消息
    recvMessage(sockFd,&t);
    if(strcmp(t.buff,"文件打开失败") == 0) {

        printf("%s\n",t.buff);
        return EXIT_FAILURE;// 打开文件错误退出

    }
    //接收的是文件名
    strcpy(name,t.buff);
    printf("name = %s\n", name);

    // 获取文件大小
    int fileSize;
    recvMessage(sockFd,&t);
    fileSize = atoi(t.buff);//收到的都是字符串类型，转为数字类型


    // 获取文件md5值
    char md5_str[MD5_STR_LEN + 1] = {0};
    recvMessage(sockFd,&t);
    strcpy(md5_str,t.buff);

    // 获取当前目录id
    int code;
    get_user_code(user_id, &code);

    // 确认虚拟文件表中是否存在，存在就不再插入表中
    int testlen;
    int ret1 = get_virtual_file_md5(name, user_id, code, md5_str,&testlen);// 获取成功文件存在返回0，获取失败文件不存在返回1
    if(ret1 != 0)//在当前虚拟目录下，没有该文件
    {
        add_virtual_file(code, name, user_id, md5_str, fileSize);// 文件不存在，在虚拟文件表中插入
    }


    // 检查文件在服务器本地是否存在，因为可能出现插入了虚拟文件表中在服务器本地却被删除了的情况
    char path[2048] = {0};
    char real_name[1024] = {0};
    int exitSize = 0;
    char buf[1000] = {0};
    int fd;
    int ret = if_local_exist(md5_str);//先根据md5查找本地存不存在
    if(ret == 0){// 文件在本地存在
        // 1. 获取文件本地真名
        get_real_file_name(md5_str, real_name);
        snprintf(path, sizeof(path), "%s%s%s", "NetDisk", "/", real_name);
        // 2. 打开文件
        fd = open(path, O_RDWR | O_CREAT, 0666); // 文件在本地存在，考虑误删了本地文件但表里的信息未更新
        ERROR_CHECK(fd, -1, "open");
        // 3. 获取文件大小已有大小
        struct stat statbuf;
        ret = fstat(fd, &statbuf);
        ERROR_CHECK(ret, -1, "fstat");
        exitSize = statbuf.st_size;
        // 4. 判断是否需要断点续传
        if(exitSize < fileSize){//需要断点续传
            sendMessage(sockFd,"需要断点续传");
        }
        else{// 不需要上传，秒传
            sendMessage(sockFd,"秒传"); // 通知客户端文件存在不需要上传
            if (ret1 != 0){ // 文件原先在虚拟文件表中不存在，链接数加1
                add_link(md5_str);
            }
            close(fd);
            return 0;//秒传结束，整个流程就是,看看虚拟文件表中有没有该表项，没有的话就添加进去
                     //有的话，根据md5码查找本地文件表中是否存在，存在的话，获取真实文件名，然后打开
                     //获取它的大小，看看和我传过来的大小是否一致，一致的话就说明秒传。如果原本虚拟表项就存在的话
                     //就不用变，但是如果不存在，在虚拟表中添加了表项，那么我本地链接数量加1
        }
    } else{// 文件在本地不存在
        sendMessage(sockFd,"文件在我云盘的本地没有，您需要上传文件");// 通知客户端文件需要上传
                                                                     // 文件不存在，需要下载到本地，且添加到本地文件表中
                                                                     // 1. 拼一个本地文件名
                                                                     // 拼接出一个本地文件名，用文件名、user_id和当前目录id，这样不同用户的同名文件，或同一用户不同文件夹中的同名文件，就不会在本地重名了
                                                                     // 这里的同名指文件名相同但文件内容可能不同
        bzero(path, sizeof(path));
        sprintf(real_name, "%s%d%d", name, user_id, code);
        // 2. 添加到本地文件表中
        add_local_file(md5_str, real_name);
        // 3. 拼出一个文件路径
        sprintf(path, "%s%s%s", "NetDisk", "/", real_name);
        fd = open(path, O_RDWR | O_CREAT, 0666); // 文件不存在就创建,这里创建了一个空的文件
        ERROR_CHECK(fd, -1, "open");

        // 4. 获取文件大小，这里肯定不存在，只是为了客户端不阻塞
        struct stat statbuf;
        ret = fstat(fd, &statbuf);
        ERROR_CHECK(ret, -1, "fstat");
        exitSize = statbuf.st_size;//这里就是0，意思需要客户端从0开始传
    }
    // 发送本地文件大小
    char message[100] = "";
    sprintf(message,"%d",exitSize);
    sendMessage(sockFd,message);//发送文件的偏移量，如果我这里没有这个文件的话，就是0，如果有，但是大小不一样，断点续传
                                //客户端需要偏移文件起始位置，然后开始上传。
    lseek(fd, 0, SEEK_END);//我这边也要偏移，从这里开始接收

    // 下载文件
    // 判断文件大小，是否一次接收
    if(fileSize >= 1073741824)// 大于1G
    {
        ftruncate(fd, fileSize); // 预设文件空洞(直接忽略原本文件已经存在的情况)
        char *p = (char *)mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        ERROR_CHECK(p, MAP_FAILED, "mmap");
        recvn(sockFd, p, fileSize);//接收这么长的文件;
        munmap(p, fileSize);
    } else{ // 小文件分次
        int total = exitSize;
        while (total < fileSize) {
            bzero(buf, sizeof(buf));
            ret = recvn(sockFd,&dataLength,sizeof(int));
            if(ret == 0)// ret为0说明对面提前断开了
            {
                printf("客户端传输中断\n");
                return EXIT_FAILURE;
                close(fd);
            }
            ret = recvn(sockFd, buf, dataLength);
            if(ret == 0){// ret为0说明对面提前断开了
                printf("客户端传输中断\n");
                return EXIT_FAILURE;
                close(fd);
            }
            write(fd, buf, dataLength);
            total += dataLength;
        }
    }
    recvMessage(sockFd,&t);
    if(strcmp(t.buff,"发送完毕") != 0){
        printf("%s\n",t.buff);
        return -1;
    }
    printf("接受完毕\n");
    //把文件同步到子服务器
   
    printf("那我要上传到子服务器\n");
    printf("我要上传的文件是:%s,真实名字是:%s\n",path,real_name);
    fd = open(path, O_RDWR | O_CREAT, 0666);
    char str1[] = "1111";
    
    unsigned short port1 = (unsigned short)strtoul(str1, NULL, 10);
    char ip[] = "192.168.2.38";
    int childFd1 = childTcpConnect(ip,port1);
    printf("1号服务器已经连接\n");
    int childFd2 = childTcpConnect(ip,(unsigned short)2222);
    printf("2号服务器已经连接\n"); 
    int childFd3 = childTcpConnect(ip,(unsigned short)3333);
    printf("3号服务器已经连接\n"); 


    sendChildServer(childFd1,fd,fileSize,real_name);
    printf("1号服务器已经同步\n"); 
    sendChildServer(childFd2,fd,fileSize,real_name);
    printf("2号服务器已经同步\n");
    sendChildServer(childFd3,fd,fileSize,real_name);
    printf("3号服务器已经同步\n");
    close(fd);
    return 0;
}

int sendChildServer(int netFd, int fd, int fileSize, char *real_name) {
    // 发送文件名
    printf("测试\n");
    CmdType type = PUTSTOKEN;
    sendn(netFd,&type,sizeof(CmdType));
    sendMessage(netFd, real_name);

    // 发送文件大小
    sendn(netFd, &fileSize, sizeof(int));

    // 使用 mmap 进行内存映射
    char *p = (char *) mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    int total = 0; // 偏移量
    int length = 1000; // 每次发送的固定长度

    // 循环发送数据
    while (total < fileSize) {
        // 计算本次发送的长度
        int sendLength = (fileSize - total) < length ? (fileSize - total) : length;

        // 发送数据长度
        if (sendn(netFd, &sendLength, sizeof(int)) == -1) {
            perror("sendn");
            return -1;
        }

        // 发送数据内容
        if (sendn(netFd, p + total, sendLength) == -1) {
            perror("sendn");
            return -1;
        }

        // 更新偏移量
        total += sendLength;
    }
    // 解除内存映射
    if (munmap(p, fileSize) == -1) {
        perror("munmap");
        return -1;
    }
    train_t t;
    recvMessage(netFd,&t);
    printf("%s\n",t.buff);
    return 0;
}

