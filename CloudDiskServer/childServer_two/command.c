#include "head.h"

int getsCommand(int sockfd){
    
    int fileNameSize = 0;
    char fileName[100] = ""; // 假设最大长度为 100
    recvn(sockfd, &fileNameSize, sizeof(int)); // 接收文件名长度
    recvn(sockfd, fileName, fileNameSize); // 接收文件名
    fileName[fileNameSize] = '\0'; // 添加字符串结束符'\0'

    printf("Received fileName: %s\n", fileName);

    int md5Length = 0;
    char md5_str[MD5_STR_LEN + 1] = ""; // 假设最大长度为 MAX_MD5_STR_LEN
    recvn(sockfd, &md5Length, sizeof(int)); // 接收 MD5 码长度
    recvn(sockfd, md5_str, md5Length); // 接收 MD5 码
    md5_str[md5Length] = '\0'; // 添加字符串结束符'\0'

    printf("Received MD5: %s\n", md5_str);

    int start = 0;
    recvn(sockfd, &start, sizeof(int)); // 接收起始值

    printf("Received start: %d\n", start);

    int size = 0;
    recvn(sockfd, &size, sizeof(int)); // 接收下载大小

    printf("Received size: %d\n", size);

    //打开实际存储表。根据md5码值去找到真实名字,通过中转服务器的过滤，此时文件一定是存在的
    char real_name[1024] = "";
    char file_path[1024] = {0};
    train_t t;
    get_real_file_name(md5_str, real_name);
    sprintf(file_path, "%s/%s", (char *) "NetDisk", real_name);

    //然后打开它。然后偏移到start
    int fd = open(file_path, O_RDWR);
    struct stat statbuf;
    fstat(fd, &statbuf);
    int fileSize = statbuf.st_size;// 获得文件总大小

    //然后传输size大小的文件,如果size超过了1Gb，一次性传输
    char *p = (char*)mmap(NULL, fileSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(size >=1073741824){
        send(sockfd, p + start, size, MSG_NOSIGNAL);
        printf("当前文件大于1Gb，用的mmap一次传送!\n");
    }else{
        // 小于1G用小火车分次传输
        int length = sizeof(t.buff);//初始是1000
                                    //BUG 我下面这个每次会一直陷入死循环，while永远成立 
                                    // while (start < start + size) {//每一次的起始位置小于总长度时
        int end = start + size;
        printf("end:%d,start:%d\n",end,start);
        while(start < end){ 
            if (size < length){//相当于最后一次传的长度等于剩下的长度
                length = size;
                sendn(sockfd,&length,sizeof(int));//发送最后一次的长度
                sendn(sockfd,p+start,length);//发送最后一段内容
                                             //BUG 没加下面这句,最后一次跳不出去，一直死循环
                start += length; // 更新 start 的值
            } else {//每一次传固定长度
                sendn(sockfd,&length,sizeof(int));//固定长度为1000
                sendn(sockfd,p + start,length);//每次发1000
                start += length;//偏移
            }

        }//循环传输结束
    }
    sendMessage(sockfd,"文件分片接收完毕，修改共享变量NUM，唤醒监视进程，然后把我们发的文件拼接在一起");
    return 0;
    }

int putsTokenCommand(int sockfd) {
    train_t t;
    char buf[1000];

    // 接收文件名，并打开文件
    char file[1024] = "";
    recvMessage(sockfd, &t);
    strcpy(file, t.buff);
    char path[1024] = "";
    sprintf(path, "%s/%s", "NetDisk", file);

    // 保存文件大小
    int fileSize;
    recvn(sockfd, &fileSize, sizeof(int));

    // 打开文件，清空原有内容
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
    ERROR_CHECK(fd, -1, "open");

    // 下载文件
    // 判断文件大小，是否一次接收
    if (fileSize >= 1073741824) { // 大于1G
        ftruncate(fd, fileSize); // 预设文件空洞
        char *p = (char *) mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        ERROR_CHECK(p, MAP_FAILED, "mmap");
        recvn(sockfd, p, fileSize); // 接收文件内容
        munmap(p, fileSize);
    } else { // 小文件分次传输
        int total = 0;
        int dataLength = 0;
        while (total < fileSize) {
            bzero(buf, sizeof(buf));
            int ret = recvn(sockfd, &dataLength, sizeof(int));
            if (ret == 0) {
                printf("客户端传输中断\n");
                close(fd);
                return -1;
            }
            ret = recvn(sockfd, buf, dataLength);
            if (ret == 0) {
                printf("客户端传输中断\n");
                close(fd);
                return -1;
            }
            write(fd, buf, dataLength);
            total += dataLength;
        }
    }
    
    sendMessage(sockfd,"同步成功");
    close(fd);
    return 0;
}

