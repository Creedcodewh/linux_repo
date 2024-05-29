#include "head.h"

int exitPipe[2];//父进程和子进程之间的通信管道

void sigHandler(int sigNum){
    printf("signum = %d\n",sigNum);
    //激活管道, 往管道中写一个1
    int one = 1;
    write(exitPipe[1], &one, sizeof(one));
}

int main(void){

    //创建匿名管道
    pipe(exitPipe);
    //fork之后，将创建了子进程
    pid_t pid = fork();
    if(pid > 0) {//父进程
        close(exitPipe[0]);//父进程关闭读端
        signal(SIGUSR1, sigHandler);
        wait(NULL);//等待子进程退出，回收其资源
        close(exitPipe[1]);
        printf("\nparent process exit.\n");
        exit(0);//父进程退出
    }
    //子进程 
    close(exitPipe[1]);//子进程关闭写端

    threadpool_t threadpool;
    memset(&threadpool, 0, sizeof(threadpool));

    //初始化线程池
    threadpoolInit(&threadpool, WORKNUM);
    //启动线程池
    threadpoolStart(&threadpool);

    //先初始化Tcp连接 用配置文件
    int listenFd;
    char configPath[100] = "./config/server_config";
    tcpInit(&listenFd, configPath);
    //创建epoll实例
    int epfd = epoll_create1(0);
    ERROR_CHECK(epfd, -1, "epoll_create1");
    //对listenfd进行监听
    addEpollReadfd(epfd, listenFd);//监听连接请求
    addEpollReadfd(epfd, exitPipe[0]);//监听退出请求
    
    //1、todo 创建客户队列并初始化
    clientQueue_t clientQueue;
    init_clientQueue(&clientQueue);
    //2、todo 初始化本地文件表（用于秒传）
    init_local_file();

    struct epoll_event * pEventArr = (struct epoll_event*)
        calloc(EPOLL_ARR_SIZE, sizeof(struct epoll_event));
   
    while(1) {
        int nready = epoll_wait(epfd, pEventArr, EPOLL_ARR_SIZE, -1);
        if(nready == -1 && errno == EINTR) {
            continue;
        } else if(nready == -1) {
            ERROR_CHECK(nready, -1, "epoll_wait");
        } else {
            //大于0
            for(int i = 0; i < nready; ++i) {
                int fd = pEventArr[i].data.fd;
                if(fd == listenFd) {//对新连接进行处理
                    int peerfd = accept(listenFd, NULL, NULL);
                    printf("\n conn %d has conneted.\n", peerfd);
                    addEpollReadfd(epfd, peerfd);
                } else if(fd == exitPipe[0]) {
                    //线程池要退出
                    int howmany = 0;
                    //对管道进行处理
                    read(exitPipe[0], &howmany, sizeof(howmany));
                    //主线程通知所有的子线程退出
                    threadpoolStop(&threadpool);
                    //子进程退出前，回收资源
                    threadpoolDestroy(&threadpool);
                    close(listenFd);
                    close(epfd);
                    close(exitPipe[0]);
                    printf("\nchild process exit.\n");
                    exit(0);
                } else {//客户端的连接的处理
                    handleMessage(fd, epfd, &threadpool.que,&clientQueue);
                }
            }
        }
   }

    return 0;
}

