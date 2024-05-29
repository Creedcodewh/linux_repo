#include "head.h"

int loginOneCommand(task_t * task){
    // 初始化需要用到的信息
    char user_name[100] = {0};// username!!

    // 从传入的信息中读取用户名和密码
    strncpy(user_name, task->data + task->preLength,task->cmdLength - task->preLength);

    // 查找用户，获取盐值和密文
    char crptpasswd[100] = {0};
    char salt[SATL_LEN + 1] = {0};
    int ret = get_user_salt(user_name, salt, crptpasswd);
    if(ret != 0){// 查找不成功

        sendMessage(task->peerfd,"该用户名未注册");
        return -1;
    }
    //查找成功，发送密文
    sendMessage(task->peerfd,salt);
    return 0;
}

int loginTwoCommand(task_t * task){
    //收到用户名和新密文
    //从根据用户名从数据库里面拿到旧密文，然和和新密文对比是否一样
    //一样则登录成功
    char user_name[100] = {0};
    char new_crptpasswd[100] = {0};
    strncpy(user_name, task->data,task->preLength);
    strncpy(new_crptpasswd,task->data + task->preLength,task->cmdLength - task->preLength);

    char salt[SATL_LEN + 1] = {0};
    char crptpasswd[100] = {0};
    get_user_salt(user_name, salt, crptpasswd);//由于是登录第二步，用户名一定存在，不用错误判断

    if(strcmp(new_crptpasswd,crptpasswd) != 0){
        sendMessage(task->peerfd,"密码错误，登录失败");
        return -1;
    }
    sendMessage(task->peerfd,"登录成功");
    //登录成功，生成当前用户的key值，然后用key值生成Token

    char pwd[100] = "~/";
    char key[100] = {0};
    char token[1024] = {0};
    int user_id = 0;
    find_user_id(user_name, &user_id);
    sprintf(key, "YoUR sUpEr S3krEt %d HMAC kEy HeRE", user_id);

    task->pclientQueue->client[task->peerfd] = user_id;// 插入用户队列，下标为netFd
                                                       //sysadd(user_id, "login successful!");// 记入日志;
    encode(key,user_name,token);//获得加密Token
    sendMessage(task->peerfd,token);
    sendMessage(task->peerfd,key);
    sendMessage(task->peerfd,pwd);
    fdAdd(task->peerfd,task->pclientQueue);//登录成功开始计时
    return 0;
}
