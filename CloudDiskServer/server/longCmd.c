#include "head.h"

int getsCommand(task_t * task){
    //先根据文件名去查询文件在当前用户user_id和当前目录code存不存在，还要判断文件类型
    //存在的话返回给用户md5码和文件大小
    //不存在的话返回给用户错误信息
    //存在的后续就是再给客户端发送三个子服务器的端口号
    int code;
    int user_id = task->pclientQueue->client[task->peerfd];//从已连接客户队列里面获取当前客户端在用户信息表中的id
    get_user_code(user_id,&code);//获取了当前用户所在文件夹的id
    char file[100] = "";
    strncpy(file,task->data + task->preLength,task->cmdLength - task->preLength);//获取文件名
    char md5_str[MD5_STR_LEN + 1] = {0};//用来存储文件的md5码值
    int filesize = 0;
    int ret = get_virtual_file_md5(file, user_id, code, md5_str,&filesize);
    if(ret != 0){
        sendMessage(task->peerfd,"文件在当前目录不存在");
        return -1;
    }
    //文件存在
    sendMessage(task->peerfd,"文件存在");
    sendMessage(task->peerfd,md5_str);//发送md5码值给客户端
    char filesize_str[100] = "";
    sprintf(filesize_str, "%d", filesize);//把整数类型的数据转换为二进制存储到字符串里面进行发送
    sendMessage(task->peerfd,filesize_str);//发送文件大小给客户端
    sendMessage(task->peerfd,"192.168.2.38");//发送子服务端的ip和端口号给客户端
    sendMessage(task->peerfd,"1111");
    sendMessage(task->peerfd,"192.168.2.38");
    sendMessage(task->peerfd,"2222");
    sendMessage(task->peerfd,"192.168.2.38");
    sendMessage(task->peerfd,"3333");
    return 0;
}

int putsCommand(task_t * task){

    //客户端先本地先打开文件，计算md5码
    //然后发送文件名和md5码
    //服务器收到以后，现在当前目录查询有没有该文件名
    //有的话返回文件名重复，没有该文件的话，就去查找实际文件存储表，看看有没有md5码一样的
    //🈶的话，在当前目录添加一个目录项（名字同传过来的文件名一样），勇士实际文件的链接数量加1
    //没有的话，就返回三个子服务器的ip端口号，然后客户端唤醒三个子线程，把三个端口号分别装进三个任务里面去
    //子线程去连接子服务器，连接成功以后，发送文件，文件名，和md5码。
    //子服务器接收完毕以后，更新实际存储表的消息，然后中转服务器更新虚拟文件表
    fdDel(task->peerfd,task->pclientQueue);
    fdAdd(task->peerfd,task->pclientQueue); 
    printf("puts命令刷新了当前用户的时间\n");
    return 0;
}

int putstokenCommand(task_t * task){

    int user_id = -1;
    char user_name[100] = {0};
    char key[100] = {0};
    char token[1024] ={0};
    strncpy(user_name,task->data,task->preLength);
    strncpy(token,task->data + task->preLength,task->cmdLength - task->preLength);
    //已经获得用户名和token
    find_user_id(user_name,&user_id);//根据传过来的名字找到用户id
                                     //然后根据用户id生成key值


    sprintf(key, "YoUR sUpEr S3krEt %d HMAC kEy HeRE", user_id);
    int ret = decode(key,user_name,token);
    if(ret != 0){
        user_id = -1;
        sendMessage(task->peerfd,"验证失败");
        return -1;
    }
    //验证成功
    sendMessage(task->peerfd,"验证成功");

    //把当前套接字task->peerfd 从监听队列里面移除去，因为后续会发文件过来
    delEpollReadfd(task->epfd,task->peerfd);
    //接收文件
    ret =  Download(task->peerfd, user_id);

    //当客户端收完了以后，会给服务器发送已经接收完毕
    //此时我需要另外启动三个子线程，它们的任务是分别连接三台子服务器
    //把我当前的接收到都文件发送给他们

    //当前这个子线程就是个究极牛马
    //把文件同步到子服务器
    
    return 0;


}
