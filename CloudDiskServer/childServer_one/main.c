#include "head.h"
//我是专门执行下载和上传任务的
//简单版，主进程去执行一切

int main(){

    //先初始化Tcp连接 用配置文件
    int listenFd;
    char configPath[100] = "./config/server_config";
    tcpInit(&listenFd, configPath);

    int epfd = epoll_create1(0);
    ERROR_CHECK(epfd, -1, "epoll_create1");
    //对listenfd进行监听
    addEpollReadfd(epfd, listenFd);//监听连接请求
    struct epoll_event * pEventArr = (struct epoll_event*)
        calloc(EPOLL_ARR_SIZE, sizeof(struct epoll_event));
    printf("1号子服务器等待任务中...\n");
    while(1) {
        int nready = epoll_wait(epfd, pEventArr, EPOLL_ARR_SIZE, -1);
        if(nready == -1 && errno == EINTR){
            continue;
        }else{
            for(int i = 0; i < nready; ++i){
                int sockfd = pEventArr[i].data.fd;
                if(sockfd == listenFd) {//对新连接进行处理){
                    int peerfd = accept(listenFd, NULL, NULL);
                    printf("\n conn %d has conneted.\n", peerfd);
                    addEpollReadfd(epfd, peerfd);
                }else{//客户端的连接连接的处理
                      //先接收类型
                    CmdType type;
                    recvn(sockfd,&type,sizeof(CmdType));
                    switch (type) {
                    case REGISTERONE:
                        // 执行 REGISTERONE 相关操作
                        break;
                    case REGISTERTWO:
                        // 执行 REGISTERTWO 相关操作
                        break;
                    case LOGINONE:
                        // 执行 LOGINONE 相关操作
                        break;
                    case LOGINTWO:
                        // 执行 LOGINTWO 相关操作
                        break;
                    case LS:
                        // 执行 LS 相关操作
                        break;
                    case CD:
                        // 执行 CD 相关操作
                        break;
                    case PWD:
                        // 执行 PWD 相关操作
                        break;
                    case REMOVE:
                        // 执行 REMOVE 相关操作
                        break;
                    case MKDIR:
                        // 执行 MKDIR 相关操作
                        break;
                    case GETS:
                        // 执行 GETS 相关操作
                        getsCommand(sockfd);
                        break;
                    case PUTS:
                        // 执行 PUTS 相关操作
                        break;
                    case PUTSTOKEN:
                        // 执行 PUTSTOKEN 相关操作
                        putsTokenCommand(sockfd);
                        break;
                    default:
                        // 默认操作
                        break;
                    }

                }
                }
            }
        }
        return 0;
    }
