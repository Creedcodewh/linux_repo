#include "head.h"

int exitPipe[2];//父进程和子进程之间的通信管道

void sigHandler(int sigNum){
    //激活管道, 往管道中写一个1
    int one = 1;
    write(exitPipe[1], &one, sizeof(one));
}
int main(){
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

    //连接服务器
    int sockFd;
    char configPath[100]="./config/client_config";
    tcpInit(&sockFd,configPath);
    printf_login((char*)"etc/one.bmp");
    printf_login((char*)"etc/open1.bmp"); 
    int flag;
    char uName[100] = {0};

    // 整个登录注册的过程
    while(1) {
        fflush(stdin);
        flag = 0;
        printf("\033[1;37m请输入1注册一个新用户");
        printf("或者输入2登录系统: \033[0m");
        fflush(stdout);
        char buf[10] = {0};
        read(STDIN_FILENO, buf, sizeof(buf));
        flag = atoi(buf);
        if(flag == 1) {
            int ret = register_user(sockFd);
            if(ret == 0) {
                printf("注册成功,请继续登录\n");
                continue;
            } else if(ret == -1) {
                printf("注册失败，请重新输入用户名:\n");
                continue;
            } else{
                printf("用户名重复，请重新输入:\n");
                continue;
            }
        }
        else if(flag == 2) {
            int ret = login(sockFd, uName);
            if(ret == -1){
                printf("\033登录失败\033[0m\n");
                continue;
            }
            else {
                break;
            }
        } else {
            printf("\033[1;37mmWrong num!\033[0m\n");
            continue;
        }
    }
    //登录成功
    printf_login((char*)"etc/login2.bmp");
    printf_login((char*)"etc/loginok.bmp");




    train_t t;
    // 获取token和key，用于子线程连接验证
    char pwd[PATH_MAX]; //用来保存用户当前所在路径，登录的时候会刷新到家目录
                        //然后cd的时候会刷新
                        //puts命令使用
    server_t serverInfo;
    bzero(&serverInfo,sizeof(server_t));
    strcpy(serverInfo.user_name,uName);
    //收到token
    recvMessage(sockFd,&t);
    strcpy(serverInfo.token,t.buf);
    //收到key值
    recvMessage(sockFd,&t);
    strcpy(serverInfo.key,t.buf);
    //登录成功，返回当前工作目录
    recvMessage(sockFd,&t);
    strcpy(pwd,t.buf);


    //gets多点下载专用
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    int NUM = 0;
    ThreadArgs args;
    bzero(&args,sizeof(ThreadArgs));
    args.mutex = &mutex;
    args.cond = &cond;
    args.NUM = &NUM;

    char input[1024] = {0};
    CmdType Type;
    char * param = NULL;
    int epfd = epoll_create(1);
    addEpollReadfd(epfd,STDIN_FILENO);// 监听标准输入
    struct epoll_event readyArr[2];
    while (1){
        printf("\33[1m\33[32m%s@工农音趴大队:%s$  \33[0m", uName, pwd);
        //printf("%s@creedcodewh:%s$ ",uName,pwd);
        fflush(stdout);
        fflush(stdin);
        int readyNum = epoll_wait(epfd, readyArr, 2, -1);
        for (int i = 0; i < readyNum; ++i){
            if (readyArr[i].data.fd == STDIN_FILENO){
                bzero(input, sizeof(input));
                read(STDIN_FILENO,input , sizeof(input));
                input[strlen(input) - 1] = 0;//把最后一位换行符去掉
                char *cmd  = strtok(input," ");
                Type = getCommandType(cmd);//此次输入的命令类型
                param  = strtok(NULL, " ");//参数值

                switch (Type) {
                case REGISTERONE:
                    // 处理 REGISTERONE 枚举值的逻辑
                    printf("注册请先退出登录\n");
                    break;
                case REGISTERTWO:
                    // 处理 REGISTERTWO 枚举值的逻辑
                    printf("注册请先退出登录\n");
                    break;
                case LOGINONE:
                    // 处理 LOGINONE 枚举值的逻辑
                    printf("您已经登录\n");
                    break;
                case LOGINTWO:
                    // 处理 LOGINTWO 枚举值的逻辑
                    printf("您已经登录\n");
                    break;
                case LS:
                    sendCmdTarin(sockFd,Type,(char*)"ls",param);
                    recvMessage(sockFd,&t);
                    if(strcmp(t.buf,"路径存在") == 0){
                        //接收搜索结果
                        recvMessage(sockFd,&t);
                        printf("%s\n",t.buf);
                    }else{
                        printf("%s\n",t.buf);
                    }
                    // 处理 LS 枚举值的逻辑
                    break;
                case CD:
                    // 处理 CD 枚举值的逻辑
                    sendCmdTarin(sockFd,Type,(char*)"cd",param);
                    recvMessage(sockFd,&t);
                    bzero(pwd, sizeof(pwd));
                    //更新pwd
                    strcpy(pwd,t.buf);
                    break;
                case PWD:
                    // 处理 PWD 枚举值的逻辑
                    sendCmdTarin(sockFd,Type,(char*)"pwd",param);
                    recvMessage(sockFd,&t);
                    printf("%s\n",t.buf);
                    break;
                case REMOVE:
                    // 处理 REMOVE 枚举值的逻辑
                    sendCmdTarin(sockFd,Type,(char*)"rm",param);
                    if(strcmp(t.buf,"删除成功")){
                        //没有消息就是最好的消息
                        continue;
                    }else{
                        printf("%s\n",t.buf);
                        break;
                    case MKDIR:
                        // 处理 MKDIR 枚举值的逻辑
                        sendCmdTarin(sockFd,Type,(char*)"mkdir",param);
                        recvMessage(sockFd,&t);
                        if(strcmp(t.buf,"创建成功") == 0){
                            continue;
                        }else{
                            printf("%s\n",t.buf);
                        }
                        break;
                    case GETS:
                        // 处理 GETS 枚举值的逻辑
                        //具体执行还是要交给子线程，但是还要给服务器发个消息，刷新一下已连接时间 

                        if(param == NULL){
                            printf("请加上下载文件名\n");
                            break;
                        }
                        handleGetsMessage(sockFd,&threadpool.que, Type, param, &args);
                        pthread_t listenerThread;
                        pthread_create(&listenerThread, NULL, threadListener, (void*)&args);
                        break;
                    case PUTS:
                        // 处理 PUTS 枚举值的逻辑
                        if(param == NULL){
                            printf("请加上传文件名n");
                            break;
                        }
                        handlePutsMessage( sockFd, &threadpool.que, Type, param , &serverInfo);
                        break;
                    case PUTSTOKEN:
                        // 处理 TOKEN 枚举值的逻辑
                        break;
                    case INVALID:
                        // 处理 INVALID 枚举值的逻辑
                        printf("command not found\n");
                        break;
                    default:
                        // 处理未知枚举值的逻辑
                        printf("command not found\n");
                        break;
                    }
                }
            }
        }
    }
} 
