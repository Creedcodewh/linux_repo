#include "head.h"
int make_salt(char *salt)
{   
    int i , flag;
    srand(time(NULL));
    salt[0] = '$';
    salt[1] = '6'; 
    salt[2] = '$'; 
    salt[SATL_LEN-1] = '$';
    for(i = 3; i < SATL_LEN - 1; ++i)
    {
        flag = rand()%3; switch(flag)
        {
        case 0:
            salt[i] = rand()%26 + 'a';
            break;
        case 1:
            salt[i] = rand()%26 + 'A';
            break;
        case 2:
            salt[i] = rand()%10 + '0';
            break;
        }
    }
    return 0;
}

int registerOneCommand(task_t * task){
    //1、注册一阶段，先检查用户名是否存在
    //2、不存在的话，生成盐值并发送给用户
    char user_name[100] = {0};
    char passwd[100] = {0};
    int user_id;
    char message[100] = "";
    strncpy(user_name,task->data+task->preLength, task->cmdLength-task->preLength);
    printf("user_name = %s\n", user_name);
    strcpy(passwd, task->data + task->preLength);

    //todo 检查用户名是否存在
    int ret = find_user_id(user_name, &user_id);
    if (ret == 0){ // 如果这里查找成功，说明用户已存在，注册失败(用户名不可重复)
        sprintf(message,"用户名重复");
        sendMessage(task->peerfd,message);
        return -1;
    }//不存在

    //制造一个随机盐值并发送给客户端
    char salt[SATL_LEN + 1] = {0};
    make_salt(salt);
    sendMessage(task->peerfd,salt);
    return 0;
}

int registerTwoCommand(task_t * task){
    //1、注册二阶段
    char user_name[50] = {0};
    char crptpasswd[100] = {0};
    strncpy(user_name,task->data,task->preLength);
    strcpy(crptpasswd,task->data+task->preLength);

    char *salt = NULL; 
    const char *start = strchr(crptpasswd, '$'); // 找到第一个'$'的位置
    if (start != NULL) {
        const char *end = strrchr(crptpasswd, '$'); // 找到最后一个'$'的位置
        if (end != NULL) {
            size_t length = end - start + 1; // 计算两个'$'之间的字符个数
            char *result = (char*)calloc(1, length + 1); // 用于存储提取的子字符串
            strncpy(result, start, length); // 将子字符串复制到result中
            result[length] = '\0'; // 添加字符串结束符
            salt= result;
            printf("提取的子字符串是：%s\n", salt);
            printf("获取的盐值为：%s\n", salt); // 在找到盐值后打印
        } else {
            printf("未找到最后一个'$'。\n");
        }
    } else {
        printf("未找到第一个'$'。\n");
    }
    printf("提取出来的密文:%s\n",salt);

    char pwd[100] = "~/";// pwd!!

    // 注册成功，插入数据库
    MYSQL *conn = NULL;
    int ret = mysqlconnect(&conn);
    if(ret != 0)
    {
        sendMessage(task->peerfd,"连接数据库失败");
    }
    ret = mysql_query(conn, "set names 'utf8'");
    if(ret != 0)
    {
        printf("error query1:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    char query[1024] = {0};
    sprintf(query, "insert into client_info(username, salt, cryptpasswd, pwd, code) values('%s', '%s', '%s', '%s', 0)", user_name, salt, crptpasswd, pwd);
    ret = do_query(query, &conn);// 插入注册信息
    if(ret != 0)
    {
        sendMessage(task->peerfd,"注册失败");
    }
    mysql_close(conn);

    //to do 把注册操作写入日记
    
    sendMessage(task->peerfd,"注册成功");
    return 0;
}









