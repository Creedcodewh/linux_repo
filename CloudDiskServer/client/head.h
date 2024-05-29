#ifndef __WD_FUNC_H                                      
#define __WD_FUNC_H

#include <stdio.h>
#include <stdlib.h>                                      
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <error.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/uio.h>
#include <sys/sendfile.h>
#include <shadow.h>
#include <mysql/mysql.h>
#include "md5.h"

#define READ_DATA_SIZE 1024
#define MD5_SIZE 16
#define MD5_STR_LEN (MD5_SIZE * 2)
#define SATL_LEN 14
#define ARGS_CHECK(argc, num)   {\
    if(argc != num){\
        fprintf(stderr, "ARGS ERROR!\n");\
        return -1;\
    }}

#define ERROR_CHECK(ret, num, msg) {\
    if(ret == num) {\
        perror(msg);\
        return -1;\
    }}

#define THREAD_ERROR_CHECK(ret, func) {\
    if(ret != 0) {\
        fprintf(stderr, "%s:%s\n", func, strerror(ret));\
    }}
#define WORKNUM 4

typedef struct BGR_S {
    unsigned char B;
    unsigned char G;
    unsigned char R;
}BGR;

int load_pic(unsigned int *width, unsigned int *height, BGR **stream, char *pic_file_name);
int e2b(unsigned char in);
void print_color(BGR bgr_f, BGR bgr_b);
int printf_login(char *argv);


// 命令枚举
typedef enum{
    REGISTERONE,//注册第一阶段
    REGISTERTWO,//注册第二阶段
    LOGINONE,//登录第一阶段
    LOGINTWO,//登录第二阶段
    LS,
    CD,
    PWD,
    REMOVE,
    MKDIR,
    GETS,
    PUTS,
    PUTSTOKEN,
    INVALID
}CmdType;
// 传输文件用的小火车
typedef struct train_s{
    int length;
    char buf[1000];
} train_t;

// 传输命令用的小火车
typedef struct tain_state_s{
    int length;
    CmdType cmdType;
    int pre_length;
    char buf[1000];
} train_cmd_t;                                                 


typedef struct server_s{// 记录服务端信息,用于上传操作
                        // 单独调用一个子线程去重新tcp连接
    char cmd[100];
    char token[1024];
    char key[100];
    char user_name[100];
} server_t;

typedef struct {
    pthread_mutex_t* mutex;
    pthread_cond_t* cond;
    int* NUM;
    char fileName[100];

} ThreadArgs;//多点下载，单独的监听函数需要使用

//任务结构体 用于上传和下载
typedef struct task_s{
    //首先子线程要去连接子服务器
    CmdType type;//执行gets操作还是puts操作
    char * fileName;

    char ip[256];//ip地址
    unsigned short port;//端口号


    //都是文件操作，那么都需要md5码
    char md5_str[MD5_STR_LEN + 1];

    //下载操作
    int start;//起始位置
    int size;//下载多大
    char target[100];//下载到目标文件名里面

    //上传操作
    int fileSize;//上传总大小

    server_t * serverInfo;
    ThreadArgs * args;

    struct task_s * pNext;//指向下一个命令的指针
}task_t;



int recvn1(int sockFd, void *pstart, int len);
int Upload(int netFd, char *file);
int Download(int sockFd);



//任务队列结构体
typedef struct task_queue_s
{
    task_t * pFront;//队头
    task_t * pRear;//队尾
    int queSize;//记录当前任务的数量
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int flag;//0 表示要退出，1 表示不退出  
}task_queue_t;

//线程池结构体
typedef struct threadpool_s {
    pthread_t * pthreads;
    int pthreadNum;
    task_queue_t que;//...任务队列
}threadpool_t; 

//任务队列
int queueInit(task_queue_t * que);//任务队列初始化
int queueDestroy(task_queue_t * que);//任务队列销毁
int queueIsEmpty(task_queue_t * que);//判断任务队列是否为空
int taskSize(task_queue_t * que);//任务队列里面任务的数量
int taskEnque(task_queue_t * que, task_t * task);//入队
task_t * taskDeque(task_queue_t * que);//出队
int broadcastALL(task_queue_t* que);//广播？

int threadpoolInit(threadpool_t *, int num);//线程池的初始化  
int threadpoolDestroy(threadpool_t *);//线程池的销毁          
int threadpoolStart(threadpool_t *);//线程池启动
int threadpoolStop(threadpool_t *);//线程池停止


int addEpollReadfd(int epfd, int fd);
int delEpollReadfd(int epfd, int fd);
int tcpInit(int *pSockFd, char *file);
int childTcpConnect(const char * ip, unsigned short port);
int encode(char *key, char *user_name, char *token);// 重新生成token值



int register_user(int sockFd);
int login(int sockFd, char *uName);
void doTask(task_t * task);
int putsCommand(task_t *task);
int getsCommand(task_t * task);
void* threadListener(void* arg);
void mergeFiles(const char* filename1, const char* filename2, const char* filename3, const char* targetFilename);

void handleGetsMessage(int sockfd, task_queue_t * que,CmdType Type,char* param,ThreadArgs * args);
void handlePutsMessage(int sockfd, task_queue_t * que,CmdType Type,char* param ,server_t * serverInfo);

int putsFile(int netFd, char *file);

char* getInput(char* input);
CmdType getCommandType(const char *cmd);
const char * getCommand(char* input,CmdType * type,char * param);


int sendn(int sockfd, const void * buff, int len);
int sendCmdTarin(int sockfd,CmdType type,char* cmd1,char *cmd2);
int sendMessage(int fd, const char *message);
int recvn(int sockfd, void * buff, int len);
int recvMessage(int sockfd,train_t * t);
#endif
