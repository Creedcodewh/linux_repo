#include "head.h" 
int register_user(int sockFd)
{
    char user_name[100] = {0};
    char passwd[100] = {0};
    train_t t;//接收消息的小火车
    printf("\033[1;37m请输入用户名:\033[0m");
    fflush(stdout);
    scanf("%s", user_name);
    printf("\033[1;37m请输入密码:\033[0m");
    fflush(stdout);
    scanf("%s", passwd);
    
    // 发送命令火车
    sendCmdTarin(sockFd,REGISTERONE,(char*) "registerstart",user_name);
    //已经发送用户名，等待客户端检查用户名是否存在。
    recvMessage(sockFd,&t);
    if(strcmp(t.buf,"用户名重复") == 0){
        return -1;//用户名重复
    }//不是这个就是用户名可用，那么t.buf里面是盐值

    //第二阶段
    //存放盐值发送密文
    char *crptpasswd = crypt(passwd, t.buf);
    // 计算出密文并发送
    sendCmdTarin(sockFd,REGISTERTWO,user_name,crptpasswd);
    recvMessage(sockFd,&t);
    if(strcmp(t.buf,"注册成功") == 0){
        return 0;//注册成功
    }
    return 1;//注册失败
}
