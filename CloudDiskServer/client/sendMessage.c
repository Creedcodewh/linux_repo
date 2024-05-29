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

int sendCmdTarin(int sockfd,CmdType type,char* cmd1,char *cmd2){
    train_cmd_t t;//声明一个命令火车
    bzero(&t,sizeof(t));//清空
    t.cmdType = type;//存储类型
    t.pre_length = strlen(cmd1);//存储第一个参数的长度
    strcat(t.buf, cmd1);//存进车厢
    if(cmd2 ==NULL){//没有参数
        t.length = strlen(t.buf);
        //火车总长度
        size_t total_length = sizeof(t.length)+ sizeof(t.cmdType)  + sizeof(t.pre_length) +  t.length;

        int ret = sendn(sockfd,&t,total_length);
        if(ret != (int)total_length){
            return -1;
        }
        return 0;
    }
    strcat(t.buf,cmd2);//参数2也存进车厢
    t.length = strlen(t.buf);//车厢总长度

    //火车总长度
    size_t total_length = sizeof(t.length)+ sizeof(t.cmdType)  + sizeof(t.pre_length) +  t.length;

    int ret = sendn(sockfd,&t,total_length);
    if(ret != (int)total_length){
        return -1;
    }
    return 0;
}

int sendMessage(int fd, const char *message){
    // 发送响应消息
    train_t t;
    memset(&t,0,sizeof(t));
    t.length = strlen(message);
    memcpy(t.buf,message,t.length);
    printf("发送的是：%s\n",t.buf);
    int ret = sendn(fd,&t,4 + t.length);
    if (ret == t.length + 4) { // 发送成功
        printf("回应发送成功\n");
    } else {
        printf("回应发送失败");
    }
    return 0;
}
