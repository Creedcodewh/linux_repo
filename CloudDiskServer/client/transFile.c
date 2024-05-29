#include "head.h"

// 自己实现MSG_WAITALL
int recvn1(int sockFd, void *pstart, int len)
{
    int total = 0;
    int ret;
    char *p = (char*)pstart;
    while(total < len)// 收完为止
    {
        ret = recv(sockFd, p + total, len - total, 0);
        if(ret == 0)// 进来说明没收完，没收完返回值却是0则说明对面断开了，可以退出
        {
            return 0;
        }
        ERROR_CHECK(ret, -1, "recv");
        total += ret;
    }
    return 0;
}

int Upload(int netFd, char *file)
{
    char file_path[1024] = {0};//文件名
    sprintf(file_path, "%s%s%s","Home", "/", file);//打开路径 拼接
    int fd = open(file_path, O_RDWR);
    if(fd == -1)// 打开文件错误需要通知对面
    {
        perror("open");
        sendMessage(netFd,"文件打开失败");
        return -1;
    }

    // 发送文件名
    sendMessage(netFd,file);

    // 发送文件总大小
    struct stat statbuf;
    int ret = fstat(fd, &statbuf);
    ERROR_CHECK(ret, -1, "fstat");
    int fileSize = statbuf.st_size;// 获得文件总大小
    char message[100] = "";
    sprintf(message,"%d",fileSize);
    sendMessage(netFd,message);//把数字存到字符串里面，然后发送，因为我的发送函数只发字符串，对端接收到后转换为数字类型即可


    // 计算并发送文件md5值(这里大文件可能耗时会比较久)
    char md5_str[MD5_STR_LEN + 1] = {0};
    Compute_file_md5(file_path, md5_str);
    sendMessage(netFd,md5_str);    

    train_t t;
    
    // 确认是否需要上传
    recvMessage(netFd,&t);
    if(strcmp(t.buf,"秒传") == 0){
        printf("%s",t.buf);
        return 0;
    }
    printf("%s\n",t.buf);
    //不是秒传，都需要上传

    //接收服务器已经存在多少
    recvMessage(netFd,&t);
    int exitSize = atoi(t.buf);

    // 发送文件内容
    // 判断文件大小，大文件用一次性传输
    char *p = (char*)mmap(NULL, fileSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    int total = exitSize;//偏移量
    if(fileSize >= 1073741824){// 大于1G
        send(netFd, p + total, fileSize, MSG_NOSIGNAL);
        printf("当前文件大于1Gb，用的mmap一次传送!\n");
    } else {
        // 小于1G用小火车分次传输
        int length = sizeof(t.buf);
        while (total < fileSize) {//每一次的起始位置小于总长度时
            if ((fileSize - total) < length){//相当于最后一次传的长度等于剩下的长度
                length = fileSize - total;
                sendn(netFd,&length,sizeof(int));//发送最后一次的长度
                sendn(netFd,p+total,length);//发送最后一段内容
                //BUG这个分支没更新total的值
                total += length;
            } else {//每一次传固定长度
                sendn(netFd,&length,sizeof(int));//固定长度为1000
                sendn(netFd,p + total,length);//每次发1000
                total += length;//偏移
             }
        }//循环传输结束
     }
    sendMessage(netFd,"发送完毕");
    close(fd);
    munmap(p, fileSize);
    return 0;
}

int Download(int sockFd)
{
    // 先获取文件名
    char name[512] = {0};
    int dataLength;
    recvn1(sockFd, &dataLength, sizeof(int));
    if(dataLength == 0)
    {
        printf("Wrong file name!\n");
        return EXIT_FAILURE;
    }
    recvn1(sockFd, name, dataLength);
    // printf("name = %s\n", name);

    char file_path[1024] = {0};
    sprintf(file_path, "%s%s%s","download", "/", name);
    int fd = open(file_path, O_RDWR | O_CREAT, 0666);// 文件不存在就创建
    ERROR_CHECK(fd, -1, "open");
    char buf[1000] = {0};

    // 获取文件大小
    int fileSize;
    recvn1(sockFd, &dataLength, sizeof(int));
    recvn1(sockFd, &fileSize, dataLength);

    // 断点重传:
    // 检查本地是否有文件存在，不考虑有不同文件但文件名相同的情况，存在则文件大小不为0
    struct stat statbuf;
    int ret = fstat(fd, &statbuf);
    ERROR_CHECK(ret, -1, "fstat");
    int exitSize = statbuf.st_size;
    send(sockFd, &exitSize, sizeof(exitSize), MSG_NOSIGNAL);
    lseek(fd, 0, SEEK_END);

    time_t timeBeg, timeEnd;

    // 获取文件内容
    // 判断文件大小，是否一次接收
    timeBeg = time(NULL);
    if(fileSize >= 1073741824)// 大于1G
    {
        ftruncate(fd, fileSize); // 预设文件空洞(直接忽略原本文件已经存在的情况)
        char *p = (char *)mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        ERROR_CHECK(p, MAP_FAILED, "mmap");
        recvn1(sockFd, p, fileSize);
        munmap(p, fileSize);        timeEnd = time(NULL);
    }
    else // 小文件分次传
    {
        while (1)
        {
            bzero(buf, sizeof(buf));
            ret = recv(sockFd, &dataLength, sizeof(int), 0);
            if (dataLength == 0)
            {
                timeEnd = time(NULL);
                break;
            }
            if (ret == 0)// 对面连接断开
            {
                close(fd);
                return EXIT_FAILURE;
            }
            recvn1(sockFd, buf, dataLength);
            write(fd, buf, dataLength);
        }
    }
    close(fd);
    int total_time = timeEnd - timeBeg;
    // printf("total time = %ds\n", total_time);
    send(sockFd, &total_time, sizeof(int), MSG_NOSIGNAL);
    return 0;
}
