#include "head.h"

//主线程调用:处理客户端发过来的消息
void handleMessage(int sockfd, int epfd, task_queue_t * que,clientQueue_t *pclientQueue)
{   
    printf("客户端 %d 发来了消息！\n", sockfd);
    //消息格式：cmdLength cmd content
    //1.1 获取消息长度
    task_t *ptask = (task_t*)calloc(1, sizeof(task_t));
    ptask->peerfd = sockfd;//套接字
    ptask->epfd = epfd;//监听队列，后面token验证需要用到
    ptask->pclientQueue = pclientQueue;//客户队列
    int ret = recvn(sockfd, &ptask->cmdLength, sizeof(ptask->cmdLength));
    printf("命令总长度为: %d\n", ptask->cmdLength);

    //1.2 获取消息类型
    ret = recvn(sockfd, &ptask->type, sizeof(ptask->type));
    printf("命令类型为: %d\n", ptask->type);

    ret = recvn(sockfd,&ptask->preLength,sizeof(ptask->preLength));
    printf("参数1的长度：%d\n",ptask->preLength);

    if(ptask->cmdLength > 0) {
        //1.3 获取参数内容
        ret = recvn(sockfd, ptask->data, ptask->cmdLength);
        if(ret > 0) {
            //往线程池中添加任务
            taskEnque(que, ptask);
        }
    } else if(ptask->cmdLength == 0){
        //没有参数
        taskEnque(que, ptask);
    }

    if(ret == 0) {//连接断开的情况
        printf("\nconn %d is closed.\n", sockfd);
        delEpollReadfd(epfd, sockfd);
        close(sockfd);
    }
}


//子线程调用:处理任务队列里面的任务
void doTask(task_t * task)
{
    assert(task);
    switch (task->type) {
    case REGISTERONE:
        printf("Performing REGISTERONE task\n");
        registerOneCommand(task);
        break;
    case REGISTERTWO:
        printf("Performing REGISTERTWO task\n");
        registerTwoCommand(task);
        break;
    case LOGINONE:
        printf("Performing LOGINONE task\n");
        loginOneCommand(task);
        break;
    case LOGINTWO:
        printf("Performing LOGINTWO task\n");
        loginTwoCommand(task);
        break;
    case LS:
        printf("Performing LS task\n");
        lsCommand(task);
        break;
    case CD:
        printf("Performing CD task\n");
        cdCommand(task);
        break;
    case PWD:
        printf("Performing PWD task\n");
        pwdCommand(task);
        break;
    case REMOVE:
        printf("Performing REMOVE task\n");
        removeCommand(task);
        break;
    case MKDIR:
        printf("Performing MKDIR task\n");
        mkdirCommand(task);
        break;
    case GETS:
        printf("Performing GETS task\n");
        getsCommand(task);
        break;
    case PUTS:
        printf("Performing PUTS task\n");
        putsCommand(task);
        break;
    case PUTSTOKEN:
        printf("Performing TOKEN task\n");
        putstokenCommand(task);
        break;
    default:
        printf("Invalid command type\n");}

}



