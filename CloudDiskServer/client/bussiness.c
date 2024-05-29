#include "head.h"
void handleGetsMessage(int sockfd, task_queue_t * que,CmdType Type,char* param,ThreadArgs * args){
    //此时此刻，我知道我是长命令，要下载或者上传文件

    train_t t;
    sendCmdTarin(sockfd,Type,(char *)"gets",param);

    recvMessage(sockfd,&t);
    if(strcmp(t.buf,"文件存在") != 0){
        printf("%s\n",t.buf);
        return ;
    }
    printf("%s\n",t.buf);

    //要实现多点下载,我主线程发送一个gets命令,带上文件名。已经ok
    //中转服务器收到消息以后，先去根据当前目录id和userid查询数据库里面有没有该文件。已经ok
    //有的话就返回md5码值和文件大小。主线程先保存。
    //同时把文件大小分为三部分，假如大小900mb,存到三个结构体里面，（md5码，起始值，大小）
    //（md5，0，300）（md5，300，300）（md5，600，300）；
    //然后服务器继续转给我三个服务器的端口，接收保存。
    //然后我唤醒三个子线程去连接三个服务器，带上key值和token？
    //服务器验证以后返回我肯定值，我接收到以后发送（md5,0,300）
    //服务器根据md5值去实际存储表中给我发送这么大小的文件，偏移。
    //我子线程收到以后再合并。

    //BUG
    //pthread_t listenerThread;
    //pthread_create(&listenerThread, NULL, threadListener, (void*)&args);

    task_t *ptask1 = (task_t*)calloc(1, sizeof(task_t));
    task_t *ptask2 = (task_t*)calloc(1, sizeof(task_t));
    task_t *ptask3 = (task_t*)calloc(1, sizeof(task_t));
    //我子线程要什么，就装进task里面去
    //1、类型
    //2、文件名
    //3、md5码
    //4、文件大小
    //5、起始值，下载大小
    //6、ip、端口
    strcpy(args->fileName,param);

    ptask1->type = Type;
    ptask2->type = Type; 
    ptask3->type = Type;


    ptask1->args = args;
    ptask2->args = args;
    ptask3->args = args;

    printf("test,文件名:%s",ptask1->args->fileName);

    ptask1->fileName = param;
    ptask2->fileName = param;
    ptask3->fileName = param;

    recvMessage(sockfd,&t);//收文件的md5码
    strcpy(ptask1->md5_str,t.buf);
    strcpy(ptask2->md5_str,t.buf);
    strcpy(ptask3->md5_str,t.buf);
    printf("文件的md5码为%s\n",ptask1->md5_str);//test

    int filesize = 0;
    recvMessage(sockfd,&t);
    filesize = atoi(t.buf);//已经接收到文件大小
    if(filesize>= 1073741824){
        //printf_login((char*)"etc/lll.bmp");
        printf_login((char*)"etc/kkk.bmp");
        printf_login((char*)"etc/5678.bmp");
    }else{
        printf_login((char*)"etc/jjj.bmp");
    }
    ptask1->fileSize = atoi(t.buf);
    ptask2->fileSize = atoi(t.buf);
    ptask3->fileSize = atoi(t.buf);


    int remainder = filesize % 3 ;
    printf("余数为：%d\n",remainder);
    //BUG
    int size = filesize / 3; 
    ptask1->start = 0;
    ptask1->size = size;
    strcpy(ptask1->target,"temporary1");

    ptask2->start = ptask1->start + ptask1->size;
    ptask2->size = size;
    strcpy(ptask2->target,"temporary2");

    ptask3->start = ptask2->start + ptask2->size;
    ptask3->size = size + remainder;
    strcpy(ptask3->target,"temporary3");

    printf("文件大小为%d\n",filesize);//test
    printf("一号服务器的下载文件的起始地址为%d,下载大小为%d,下载到临时文件%s\n", ptask1->start,ptask1->size,ptask1->target);
    printf("二号服务器的下载文件的起始地址为%d,下载大小为%di,下载到临时文件%s\n", ptask2->start,ptask2->size,ptask2->target);
    printf("三号号服务器的下载文件的起始地址为%d,下载大小为%di,下载到临时文件%s\n", ptask3->start,ptask3->size,ptask3->target);

    recvMessage(sockfd,&t);//ip1
    strcpy(ptask1->ip,t.buf); 
    recvMessage(sockfd,&t);//端口1
    ptask1->port = (unsigned short)strtoul(t.buf, NULL, 10);
    printf("1号ip:%s,端口为:%d\n",ptask1->ip,ptask1->port);

    recvMessage(sockfd,&t);//ip2
    strcpy(ptask2->ip,t.buf);
    recvMessage(sockfd,&t);//端口2
    ptask2->port = (unsigned short)strtoul(t.buf, NULL, 10); 
    printf("2号ip:%s,端口为:%d\n",ptask2->ip,ptask2->port);

    recvMessage(sockfd,&t);//ip3
    strcpy(ptask3->ip,t.buf);
    recvMessage(sockfd,&t);//端口3
    ptask3->port = (unsigned short)strtoul(t.buf, NULL, 10); 
    printf("3号ip:%s,端口为:%d\n",ptask3->ip,ptask3->port);
    //三个任务已经初始化完毕，放进任务队列，让子线程去处理

    taskEnque(que, ptask1);
    taskEnque(que, ptask2);
    taskEnque(que, ptask3);



}

void handlePutsMessage(int sockfd, task_queue_t * que,CmdType Type,char* param ,server_t * serverInfo){
    //此时我还是主线程要执行puts命令
    //我本身还是要发个消息过去刷新一下时间，要不然把我给踢掉了
    //这个命令火车是对面的主线程收，然后让子线程给我刷新一下时间就可以了
    sendCmdTarin(sockfd,Type,(char *)"puts",param);
    task_t *ptask = (task_t*)calloc(1, sizeof(task_t));

    ptask->type = Type;
    ptask->fileName = param;//子线程要发文件名过去
    ptask->serverInfo = serverInfo;

    taskEnque(que,ptask);

}


void* threadListener(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    pthread_mutex_t* mutex = args->mutex;
    pthread_cond_t* cond = args->cond;
    int* NUM = args->NUM;
    printf("我监工开始监听了\n");
    while (1) {
        pthread_mutex_lock(mutex);
        while (*NUM < 3) {
            pthread_cond_wait(cond, mutex);
        }
        printf("子线程下载任务 完成数量 等于 3，开始合并操作...\n");
        // 执行合并操作
        char path[1024] = "";
        sprintf(path,"%s/%s" ,"Download",args->fileName);
        const char* filename1 = "Download/temporary1";
        const char* filename2 = "Download/temporary2";
        const char* filename3 = "Download/temporary3";
        const char* targetFilename = path; 
        mergeFiles(filename1, filename2, filename3, targetFilename);
        // 重置 NUM
        *NUM = 0;
        pthread_mutex_unlock(mutex);
    }

    return NULL;
}
void mergeFiles(const char* filename1, const char* filename2, const char* filename3, const char* targetFilename) {
    FILE *file1, *file2, *file3, *targetFile;
    int ch;

    // 打开三个临时文件和目标文件
    file1 = fopen(filename1, "rb");
    file2 = fopen(filename2, "rb");
    file3 = fopen(filename3, "rb");
    targetFile = fopen(targetFilename, "wb");

    // 检查文件是否成功打开
    if (file1 == NULL || file2 == NULL || file3 == NULL || targetFile == NULL) {
        printf("无法打开文件\n");
        exit(EXIT_FAILURE);
    }

    // 将临时文件1的内容写入目标文件
    while ((ch = fgetc(file1)) != EOF) {
        fputc(ch, targetFile);
    }

    // 将临时文件2的内容写入目标文件
    while ((ch = fgetc(file2)) != EOF) {
        fputc(ch, targetFile);
    }

    // 将临时文件3的内容写入目标文件
    while ((ch = fgetc(file3)) != EOF) {
        fputc(ch, targetFile);
    }

    // 关闭所有文件句柄
    fclose(file1);
    fclose(file2);
    fclose(file3);
    fclose(targetFile);

    // 删除临时文件
    //remove(filename1);
    //remove(filename2);
    //remove(filename3);

    printf("文件合并完成\n");
}


//多点下载，我是其中一个子线程，我想要的一切数据都在task里面
//我只需要完成我的那一部分就行
int getsCommand(task_t *task){
    //先去连接子服务器
    int childFd = childTcpConnect(task->ip , task->port);
    //连接成功;

    //先发type，因为我子服务器有两个操作，提供下载和更新本地文件库
    sendn(childFd,&task->type,sizeof(CmdType));

    //发文件名长度
    int fileNameSize = strlen(task->fileName);
    sendn(childFd,&fileNameSize,sizeof(int));
    //发送文件名
    sendn(childFd,task->fileName,strlen(task->fileName));

    //发送md5码长度
    int md5Length = strlen(task->md5_str);
    sendn(childFd,&md5Length,sizeof(int));
    //发送md5码
    sendn(childFd,task->md5_str,strlen(task->md5_str));

    //发送起始值
    sendn(childFd,&task->start,sizeof(int));

    //发送下载大小
    sendn(childFd,&task->size,sizeof(int));

    //接收文件
    //我要保存在下载目录下的目标文件里面
    char path[1024] = "";
    char buf[1000] = "";
    int dataLength;
    sprintf(path, "%s/%s", "Download",task->target);
    // 打开文件，如果不存在则创建，如果存在则清空内容
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("open");
        return -1;
    }
    if(task->size >= 1073741824)// 大于1G
    {
        ftruncate(fd, task->size); // 预设文件空洞(直接忽略原本文件已经存在的情况)
        char *p = (char *)mmap(NULL, task->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        ERROR_CHECK(p, MAP_FAILED, "mmap");
        recvn(childFd, p, task->size);//接收这么长的文件;
        munmap(p, task->size);
    } else{ // 小文件分次传
        int total = 0;
        while(total < task->size){
            bzero(buf, sizeof(buf));
            int ret = recvn(childFd,&dataLength,sizeof(int));
            if(ret == 0)// ret为0说明对面提前断开了
            {
                printf("客户端传输中断\n");
                close(fd);
                return EXIT_FAILURE;
            }
            ret = recvn(childFd, buf, dataLength);
            if(ret == 0){// ret为0说明对面提前断开了
                printf("客户端传输中断\n");
                close(fd);
                return EXIT_FAILURE;
            }
            write(fd,buf,dataLength);
            total += dataLength;
        }
    }
    //分段文件接收完毕
    train_t t;
    recvMessage(childFd,&t);
    printf("%s\n",t.buf);
    //修改NUM
    int * num = task->args->NUM;
    pthread_mutex_lock(task->args->mutex);
    (*num)++;
    printf("我能修改NUM%d",*num);
    pthread_mutex_unlock(task->args->mutex);
    pthread_cond_signal(task->args->cond); 
    return 0;
}
int putsCommand(task_t *task){

    int sockFd_child;
    char configPath[100]="./config/client_config";
    tcpInit(&sockFd_child,configPath );

    train_t t;
    //发送PUTS命令火车，里面有用户名和token
    sendCmdTarin(sockFd_child,PUTSTOKEN,task->serverInfo->user_name,task->serverInfo->token);
    //一收一发
    recvMessage(sockFd_child,&t);
    if(strcmp(t.buf,"验证成功") != 0){
        //验证失败
        printf("验证失败\n");

        //第二次尝试
        encode(task->serverInfo->key,task->serverInfo ->user_name,task->serverInfo->token);
        sendCmdTarin(sockFd_child,PUTSTOKEN,task->serverInfo->user_name,task->serverInfo->token);
        recvMessage(sockFd_child,&t);
        if(strcmp(t.buf,"验证成功") != 0){
            printf("彻底失败，GG\n");
            close(sockFd_child);
            return -1;
        }
    }
    //验证成功，此时服务端已经把我从监听队列里面移除
    //后面我发的消息，主线程已经收不到了
    //那一个子线程全程为我服务，哦耶我是vip咯

    char fileName[100]= "";
    strcpy(fileName,task->fileName);

    // 根据文件名去打开并上传文件

    Upload(sockFd_child, fileName);
    close(sockFd_child);

    return 0;
}

void doTask(task_t *task) {
    assert(task);

    switch (task->type) {
    case REGISTERONE:
        printf("Performing REGISTERONE task\n");
        // Add code for REGISTERONE task
        break;
    case REGISTERTWO:
        printf("Performing REGISTERTWO task\n");
        // Add code for REGISTERTWO task
        break;
    case LOGINONE:
        printf("Performing LOGINONE task\n");
        // Add code for LOGINONE task
        break;
    case LOGINTWO:
        printf("Performing LOGINTWO task\n");
        // Add code for LOGINTWO task
        break;
    case LS:
        printf("Performing LS task\n");
        // Add code for LS task
        break;
    case CD:
        printf("Performing CD task\n");
        // Add code for CD task
        break;
    case PWD:
        printf("Performing PWD task\n");
        // Add code for PWD task
        break;
    case REMOVE:
        printf("Performing REMOVE task\n");
        // Add code for REMOVE task
        break;
    case MKDIR:
        printf("Performing MKDIR task\n");
        // Add code for MKDIR task
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
        printf("Performing PUTSTOKEN task\n");
        // Add code for PUTSTOKEN task
        break;
    case INVALID:
        printf("Invalid task type\n");
        break;
    }
}
