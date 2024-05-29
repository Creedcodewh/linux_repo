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
#include <mysql/mysql.h>

#define SATL_LEN 12// 盐值长度
#define MD5_SIZE 16
#define MD5_STR_LEN (MD5_SIZE * 2)
#define SIZE(a) (sizeof(a)/sizeof(a[0]))

typedef void (*sighandler_t)(int);

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

#define EPOLL_ARR_SIZE 100
#define WORKNUM 3
#define ZERO 0
#define CLIENT_NUM 50// 客户端表大小
#define TIME_SLICE 300// 时间片大小
                      // 定义命令枚举类型

typedef enum{
    REGISTERONE,
    REGISTERTWO,
    LOGINONE,
    LOGINTWO,
    LS,
    CD,
    PWD,
    REMOVE,
    MKDIR,
    GETS,
    PUTS,
    PUTSTOKEN,
}CmdType;

//小火车结构体
typedef struct {
    int len;//记录内容长度
    char buff[1000];//记录内容本身
}train_t;

// 传输命令用的小火车                                                                   
typedef struct tain_cmd_s{
    int length;
    CmdType cmdType;  
    int pre_length;// 如果是注册和登录，用于记录用户名长度，如果是命令，用于记录命令部分
    char buf[1000];
} train_cmd_t;

// 定义客户端槽节点结构体
typedef struct slotNode_s {
    int netFd; // 网络文件描述符
    struct slotNode_s* next; // 下一个节点指针
} slotNode_t;

// 定义客户端槽链表结构体
typedef struct slotList_s {
    slotNode_t* head; // 链表头指针
    slotNode_t* tail; // 链表尾指针
    int size; // 链表大小
} slotList_t;

// 定义客户端队列结构体
typedef struct clientQueue_s {
    int client[CLIENT_NUM]; // 记录每个客户端的user_id
    slotList_t time_out[TIME_SLICE]; // 循环队列
    int index[CLIENT_NUM]; // 每个netFd当前在队列的位置，下标netFd对应
    int timer; // 记录该查询队列那个下标，初始为0
} clientQueue_t;

// 初始化客户端队列
int init_clientQueue(clientQueue_t *pclientQueue);

// 将网络文件描述符添加到客户端队列中
int fdAdd(int netFd, clientQueue_t *pclientQueue);

// 从客户端队列中删除指定的网络文件描述符
int fdDel(int netFd, clientQueue_t *pclientQueue);


//任务结构体
typedef struct task_s{
    int peerfd;//哪一个客户端
    int cmdLength;//参数的长度
    CmdType type;//命令类型
    int preLength;//可以理解为命令的长度/或者参数1的长度
    char data[1000];
}task_t;


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

int tcpInit(int *pSockFd, char *file);//Tcp的连接
int addEpollReadfd(int epfd, int fd);//加入监听队列
int delEpollReadfd(int epfd, int fd);//删除监听队列
int transferFile(int sockfd);//文件传输
int sendn(int sockfd, const void * buff, int len);
int sendMessage(int fd, const char *message);
int recvn(int sockfd, void * buff, int len);
int  recvMessage(int sockfd,train_t * t);

//处理客户端发过来的消息                                                        
void handleMessage(int sockfd, int epfd, task_queue_t * que,clientQueue_t *pclientQueue);
void doTask(task_t * task); 
int registerOneCommand(task_t * task);
int registerTwoCommand(task_t * task);
int loginOneCommand(task_t * task);
int loginTwoCommand(task_t * task);
int encode(char *key, char *user_name, char *token);//加密
int decode(char *key, char *user_name, char *token);//解密
int cdCommand(task_t * task);
int lsCommand(task_t * task);
int mkdirCommand(task_t * task);
int pwdCommand(task_t * task);
int removeCommand(task_t * task);
//int getsCommand(task_t * task);
int getsCommand(int sockfd);
int putsTokenCommand(int sockfd);

int mysqlconnect(MYSQL **pconn);
int do_query(char *query, MYSQL **pconn);
int find_user_id(char *user_name, int *puser_id);
int get_user_salt(char *user_name, char *salt, char *crptpasswd);
int get_user_code(int user_id, int *pcode);
int get_user_pwd(int user_id, char *pwd);
int get_parent_id(int id, int *pparent_code);
int get_file_code(int user_id, char *filename, int *pcode, int parent_id, char *type);
int update_user_pwd(int user_id, char *pwd, int code);
int find_code_files(int netFd, int code, int user_id);
int add_virtual_dir(int parent_id, char *filename, int owner_id);
int del_virtual_file(int code, int user_id);
int get_virtual_file_md5(char *file, int user_id, int parent_id, char *md5_str, int* filesize);
int get_real_file_name(char *md5_str, char *real_file_name);



#endif
