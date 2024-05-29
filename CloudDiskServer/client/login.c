#include "head.h"

int login(int sockFd, char *uName)
{
    char user_name[100] = {0};
    train_t t;//接收消息的小火车
              // 读取用户名
    printf("\033[1;37m请输入您的用户名:\033[0m");
    fflush(stdout);
    scanf("%s", user_name);

    //发送用户名看看注册了没
    sendCmdTarin(sockFd,LOGINONE,(char*)"loginstart",user_name);
    //接收回应。要么是用户名未登录，要么就是盐值
    recvMessage(sockFd,&t);
    if(strcmp(t.buf,"该用户名未注册") == 0){
        printf("该用户名未注册");
        return -1;
    }

    //第二阶段
    //接收到的是盐值，把盐值和密码加密得到密文，发给服务器
    char passwd[100] = {0};
    fflush(stdout);
    printf("\033[1;37m请输入您的密码:\033[0m");
    scanf("%s",passwd);
    
    //将密码和盐值加密得到新密文，发送给服务器判断
    char *crptpasswd = crypt(passwd, t.buf);
    sendCmdTarin(sockFd,LOGINTWO,user_name,crptpasswd);
    recvMessage(sockFd,&t);
    if(strcmp(t.buf,"密码错误") == 0){
        return -1;
    }
    //登录成功
    strcpy(uName, user_name);
    return 0;
}



