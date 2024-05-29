#include "head.h"

int recvn(int sockfd, void * buff, int len)
{
    int left = len;//还剩下多少个字节需要接收
    char * pbuf = (char*)buff;
    int ret = -1;
    while(left > 0) {
        ret = recv(sockfd, pbuf, left, 0);
        if(ret == 0) {
            break;
        } else if(ret < 0) {
            perror("recv");
            return -1;
        }

        left -= ret;
        pbuf += ret;
    }
    //当退出while循环时，left的值等于0
    return len - left;
}

//接收命令
int  recvMessage(int sockfd,train_t * t){
    memset(t, 0, sizeof(train_t));

    // 1、接收长度,并写到小火车里面;
    recvn(sockfd,&t->length,sizeof(t->length));
    // 2、接收命令数据
    int ret = recvn(sockfd, &t->buf, t->length);
    if (ret == t->length) {
        return 0;
    }
    return -1;
}
