#include "head.h"

//主线程连接主服务器
int tcpInit(int *pSockFd, char *file) {
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }

    char ip[1024] = {0};
    char port[1024] = {0};
    char buf[1024] = {0};

    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fclose(fp);
        fprintf(stderr, "Error reading IP from file\n");
        return -1;
    }

    int i = 0;
    while (buf[i] < '0' || buf[i] > '9') {
        ++i;
    }
    strcpy(ip, buf + i);
    printf("ip:%s",ip);

    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fclose(fp);
        fprintf(stderr, "Error reading port from file\n");
        return -1;
    }

    i = 0;
    while (buf[i] < '0' || buf[i] > '9') {
        ++i;
    }
    strcpy(port, buf + i);
    printf("port:%s",port);
    fclose(fp);

    *pSockFd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(atoi(port));
    connect(*pSockFd,(struct sockaddr*)&addr,sizeof(addr));
    return 0;

}   


//子线程去连接子服务器
int  childTcpConnect(const char * ip, unsigned short port)
{
    //1. 创建TCP的客户端套接字
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd < 0) {
        perror("socket");
        return -1;
    }

    //2. 指定服务器的网络地址
    struct sockaddr_in serveraddr;
    //初始化操作,防止内部有脏数据
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;//指定IPv4
    serveraddr.sin_port = htons(port);//指定端口号
                                      //指定IP地址
    serveraddr.sin_addr.s_addr = inet_addr(ip);

    //3. 发起建立连接的请求
    int ret = connect(clientfd, (const struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if(ret < 0) {
        perror("connect");
        close(clientfd);
        return -1;
    }
    return clientfd;
}
