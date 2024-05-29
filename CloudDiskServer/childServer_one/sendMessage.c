#include "head.h"
int sendn(int sockfd, const void *buff, int len) {
    int left = len;
    const char *pbuf = (const char *)buff; // 将指针声明为 const void *
    int ret = -1;
    while (left > 0) {
        ret = send(sockfd, pbuf, left, 0);
        if (ret < 0) {
            perror("send");
            return -1;
        }

        left -= ret;
        pbuf += ret;
    }
    return len - left;
}

int sendMessage(int fd, const char *message){
    // 发送响应消息
    train_t t;
    memset(&t,0,sizeof(t));
    t.len = strlen(message);
    memcpy(t.buff,message,t.len);
    printf("发送的是：%s\n",t.buff);
    int ret = sendn(fd,&t,4 + t.len);
    if (ret == t.len + 4) { // 发送成功
        printf("回应发送成功\n");
    } else {
        printf("回应发送失败");
    }
    return 0;
}
