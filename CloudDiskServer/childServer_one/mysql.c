#include "head.h"
// 连接数据库
int mysqlconnect(MYSQL **pconn) {
    char host[] = "localhost"; // 设置数据库主机名
    char user[] = "root"; // 设置数据库用户名
    char passwd[] = "1234"; // 设置数据库密码
    char database[] = "net_disk"; // 设置数据库名称

    *pconn = mysql_init(NULL); // 初始化 MySQL 连接对象
    if (*pconn == NULL) { // 检查初始化是否成功
        printf("Initialization failed: %s\n", mysql_error(*pconn));
        return EXIT_FAILURE;
    }

    // 尝试连接数据库
    if (mysql_real_connect(*pconn, host, user, passwd, database, 0, NULL, 0) == NULL) {
        printf("Connection failed: %s\n", mysql_error(*pconn));
        mysql_close(*pconn); // 连接失败时需要释放资源
        return EXIT_FAILURE;
    }

    return 0;
}

// 执行查询操作
int do_query(char *query, MYSQL **pconn) {
    int ret = mysql_query(*pconn, query); // 执行 SQL 查询
    if (ret != 0) {
        printf("Query failed: %s\n", mysql_error(*pconn));
        return EXIT_FAILURE;
    }
    return 0;
}

// 根据用户名获取用户id
int find_user_id(char *user_name, int *puser_id) {
    MYSQL *conn;
    int ret = mysqlconnect(&conn); // 连接数据库
    if (ret != 0) {
        return EXIT_FAILURE;
    }

    ret = mysql_query(conn, "set names 'utf8'"); // 把名字字符集设置为 utf8 类型
    if (ret != 0) {
        printf("Query failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    char query[] = "select username, id from client_info"; // 要执行的查询操作
    ret = do_query(query, &conn); // 执行查询操作
    if (ret != 0) {
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    MYSQL_RES *result = mysql_use_result(conn); // 获取查询结果。
    if (result == NULL) {
        printf("Result fetching failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL) { // 遍历查询结果
        if (strcmp(row[0], user_name) == 0) { // 检查用户名是否匹配
            *puser_id = atoi(row[1]); // 将用户 ID 转换为整数并存储到 puser_id 指向的变量中
            mysql_free_result(result); // 释放查询结果
            mysql_close(conn); // 关闭数据库连接
            return 0; // 返回成功状态
        }
    }

    mysql_free_result(result); // 释放查询结果
    mysql_close(conn); // 关闭数据库连接
    return EXIT_FAILURE; // 返回失败状态
}

// 根据用户名获取盐值和密文
int get_user_salt(char *user_name, char *salt, char *crptpasswd) {
    MYSQL *conn;
    int ret = mysqlconnect(&conn); // 连接数据库
    if (ret != 0) {
        return EXIT_FAILURE;
    }
    ret = mysql_query(conn, "set names 'utf8'"); // 设置字符集为utf8
    if (ret != 0) {
        printf("error query1:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    char query[]= "select username, salt, cryptpasswd from client_info";
    ret = do_query(query, &conn); // 查询用户名、盐值和密文
    if (ret != 0) {
        return EXIT_FAILURE;
    }
    MYSQL_RES *result = mysql_use_result(conn);
    if (result == NULL) {
        printf("error result: %s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL) {
        if (strcmp(row[0], user_name) == 0) {//返回结果如果和传入参数一样
            strcpy(salt, row[1]); // 将查询到的盐值赋值给salt
            strcpy(crptpasswd, row[2]); // 将查询到的密文赋值给crptpasswd
            mysql_free_result(result);
            mysql_close(conn);
            return 0;
        }
    }
    mysql_free_result(result);
    mysql_close(conn);
    return EXIT_FAILURE;
}

// 通过用户ID获取当前目录的文件夹编号
int get_user_code(int user_id, int *pcode)
{
    MYSQL *conn;
    // 连接数据库
    int ret = mysqlconnect(&conn);
    if (ret != 0)
    {
        return EXIT_FAILURE;
    }
    // 查询客户信息表，获取文件夹编号和用户ID
    char query[] = "select code, id from client_info"; 
    ret = do_query(query, &conn);
    if (ret != 0)
    {
        return EXIT_FAILURE;
    }
    // 获取查询结果
    MYSQL_RES *result = mysql_use_result(conn);
    if (result == NULL)
    {
        printf("error result: %s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    MYSQL_ROW row;
    // 遍历查询结果，寻找与用户ID匹配的记录
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        if (user_id == atoi(row[1]))
        {
            // 找到对应的用户ID，获取文件夹编号
            *pcode = atoi(row[0]);
            // 释放结果集和关闭数据库连接
            mysql_free_result(result);
            mysql_close(conn);
            return 0;
        }
    }
    // 释放结果集和关闭数据库连接
    mysql_free_result(result);
    mysql_close(conn);
    // 没有找到对应用户ID，返回失败
    return EXIT_FAILURE;
}

// 通过用户ID获取当前工作目录
int get_user_pwd(int user_id, char *pwd)
{
    MYSQL *conn;
    // 连接数据库
    int ret = mysqlconnect(&conn);
    if (ret != 0)
    {
        return EXIT_FAILURE;
    }
    // 设置字符集为utf8
    ret = mysql_query(conn, "set names 'utf8'");
    if (ret != 0)
    {
        printf("error query1:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    // 查询客户信息表，获取密码和用户ID
    char query[] = "select pwd, id from client_info";
    ret = do_query(query, &conn);
    if (ret != 0)
    {
        return EXIT_FAILURE;
    }
    // 获取查询结果
    MYSQL_RES *result = mysql_use_result(conn);
    if (result == NULL)
    {
        printf("error result: %s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    MYSQL_ROW row;
    // 遍历查询结果，寻找与用户ID匹配的记录
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        if (user_id == atoi(row[1]))
        {
            // 找到对应的用户ID，获取密码
            strcpy(pwd, row[0]);
            // 释放结果集和关闭数据库连接
            mysql_free_result(result);
            mysql_close(conn);
            return 0;
        }
    }
    // 释放结果集和关闭数据库连接
    mysql_free_result(result);
    mysql_close(conn);
    // 没有找到对应用户ID，返回失败
    return EXIT_FAILURE;
}

// 获取父节点的ID
int get_parent_id(int id, int *pparent_code)
{
    MYSQL *conn;
    // 连接数据库
    int ret = mysqlconnect(&conn);
    if (ret != 0)
    {
        return EXIT_FAILURE;
    }
    // 设置字符集为utf8
    ret = mysql_query(conn, "set names 'utf8'");
    if (ret != 0)
    {
        printf("error query1:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    // 构建查询语句
    char query[1024] = {0};
    sprintf(query, "select parent_id from virtual_file where id = %d", id);
    // 执行查询
    ret = do_query(query, &conn);
    if (ret != 0)
    {
        return EXIT_FAILURE;
    }
    // 获取查询结果
    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL)
    {
        printf("error result: %s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    // 获取结果集中的行数
    int rows = mysql_num_rows(result);
    // 如果查询结果为空，文件不存在，返回1
    if (rows == 0)
    {
        mysql_free_result(result);
        mysql_close(conn);
        return 1;
    }
    // 获取查询结果的一行数据
    MYSQL_ROW row;
    if ((row = mysql_fetch_row(result)) == NULL)
    {
        return EXIT_FAILURE;
    }
    // 获取父节点的ID并赋值给pparent_code
    *pparent_code = atoi(row[0]);
    // 释放结果集和关闭数据库连接
    mysql_free_result(result);
    mysql_close(conn);
    return 0;
}

// 根据文件名和用户ID获取文件在虚拟文件系统中的ID、父节点ID和类型
int get_file_code(int user_id, char *filename, int *pcode, int parent_id, char *type)
{
    MYSQL *conn;
    // 连接数据库
    int ret = mysqlconnect(&conn);
    if (ret != 0)
    {
        return EXIT_FAILURE;
    }
    // 设置字符集为utf8
    ret = mysql_query(conn, "set names 'utf8'");
    if (ret != 0)
    {
        printf("error query1:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    // 构建查询语句
    char query[1024] = {0};
    sprintf(query, "select id, parent_id, type from virtual_file where owner_id = %d and filename like '%s' and parent_id = %d", user_id, filename, parent_id);
    // 执行查询
    ret = do_query(query, &conn);
    if (ret != 0)
    {
        return EXIT_FAILURE;
    }
    // 获取查询结果
    MYSQL_RES *result = mysql_use_result(conn);
    if (result == NULL)
    {
        printf("error result: %s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }

    MYSQL_ROW row;
    // 如果查询结果为空，直接返回
    if ((row = mysql_fetch_row(result)) == NULL)
    {
        return EXIT_FAILURE;
    }
    // 获取文件ID、父节点ID和类型，并赋值给相应的变量
    *pcode = atoi(row[0]);
    //优化。没有保存父亲节点
    strcpy(type, row[2]);
    // 释放结果集和关闭数据库连接
    mysql_free_result(result);
    mysql_close(conn);
    return 0;
}

// 通过用户 ID 更新用户当前目录
int update_user_pwd(int user_id, char *pwd, int code) {
    MYSQL *conn;
    // 连接数据库
    int ret = mysqlconnect(&conn);
    if(ret != 0) {
        return EXIT_FAILURE;
    }
    // 设置字符集为 utf8
    ret = mysql_query(conn, "set names 'utf8'");
    if(ret != 0) {
        printf("error query1:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    // 更新密码
    char query[1024] = {0};
    sprintf(query, "update client_info set pwd = '%s' where id = %d", pwd, user_id);
    ret = do_query(query, &conn);
    if(ret != 0) {
        return EXIT_FAILURE;
    }
    // 更新当前目录编号
    bzero(query, sizeof(query));
    sprintf(query, "update client_info set code = %d where id = %d", code, user_id);
    ret = do_query(query, &conn);
    if(ret != 0) {
        return EXIT_FAILURE;
    }

    mysql_close(conn);
    return EXIT_FAILURE;
}

// 增加虚拟文件夹
int add_virtual_dir(int parent_id, char *filename, int owner_id) {
    MYSQL *conn;
    int ret = mysqlconnect(&conn); // 建立数据库连接
    if(ret != 0) {
        return EXIT_FAILURE; // 连接失败，返回失败状态
    }

    ret = mysql_query(conn, "set names 'utf8'"); // 设置数据库连接字符集为UTF-8
    if(ret != 0) {
        printf("error query1:%s\n", mysql_error(conn)); // 打印错误信息
        return EXIT_FAILURE;
    }

    char query[1024] = {0}; // 创建SQL查询语句缓冲区
                            // 构建SQL插入语句，向virtual_file表中插入新的虚拟文件夹记录
    sprintf(query, "insert into virtual_file(parent_id, filename, owner_id, md5, filesize, type) \
            values(%d, '%s', %d, '0', 0, 'd')", parent_id, filename, owner_id);

    // 执行SQL查询
    ret = do_query(query, &conn);
    if(ret != 0) {
        return EXIT_FAILURE; // 执行查询失败，返回失败状态
    }

    mysql_close(conn); // 关闭数据库连接
    return 0; // 返回成功状态
}

int find_code_files(int netFd, int code, int user_id){
    MYSQL *conn;
    int ret = mysqlconnect(&conn);
    if(ret != 0)
    {
        return EXIT_FAILURE;
    }
    ret = mysql_query(conn, "set names 'utf8'");
    if(ret != 0)
    {
        printf("error query1:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    char query[1024] = {0};
    //printf("当前目录的父亲id为%d  属于%d",code, user_id);//test
    sprintf(query, "select filename from virtual_file where parent_id = %d and owner_id = %d", code, user_id);
    ret = do_query(query, &conn);
    if(ret != 0)
    {
        return EXIT_FAILURE;
    }
    MYSQL_RES *result = mysql_use_result(conn);//我这里获得查询结果
    if(result == NULL)
    {
        printf("error result: %s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }

    MYSQL_ROW row;
    char filename[1024] = "";
    int first_row = 1; // 用于检查是否是第一行     

    while ((row = mysql_fetch_row(result)) != NULL) {
        if (!first_row) {
            strcat(filename, " "); // 在非第一行之前添加一个空格
        } else {
            first_row = 0; // 设置为0以指示不是第一行
        }
        strcat(filename, row[0]); // 将当前行添加到 filename 中
    }
    //printf("当前目录下面的路径有%s\n",filename);//test
    sendMessage(netFd,filename);
    mysql_free_result(result);
    mysql_close(conn);
    return EXIT_FAILURE;
}

// 删除虚拟文件
int del_virtual_file(int code, int user_id) {
    MYSQL *conn;
    // 连接数据库
    int ret = mysqlconnect(&conn);
    if(ret != 0) {
        return EXIT_FAILURE;
    }
    // 设置字符编码
    ret = mysql_query(conn, "set names 'utf8'");
    if(ret != 0) {
        printf("error query1:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    char query[1024] = {0};
    // 删除指定id的文件
    sprintf(query, "delete from virtual_file where id = %d", code);
    ret = do_query(query, &conn);
    if(ret != 0) {
        return EXIT_FAILURE;
    }
    // 删除指定用户和父节点的文件
    bzero(query, sizeof(query));
    sprintf(query, "delete from virtual_file where parent_id = %d and owner_id = %d", code, user_id);
    ret = do_query(query, &conn);
    if(ret != 0) {
        return EXIT_FAILURE;
    }
    mysql_close(conn);
    return 0;
}



// 获取文件md5值和文件大小也可以检查文件是否存在
int get_virtual_file_md5(char *file, int user_id, int parent_id, char *md5_str, int* filesize) {
    MYSQL *conn;
    int ret = mysqlconnect(&conn);
    if (ret != 0) {
        printf("连接数据库失败\n");
        return EXIT_FAILURE;
    }

    ret = mysql_query(conn, "set names 'utf8'");
    if (ret != 0) {
        printf("设置字符集失败：%s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    char query[1024] = {0};
    sprintf(query, "SELECT md5, filesize FROM virtual_file WHERE owner_id = %d AND filename LIKE '%s' AND parent_id = %d", user_id, file, parent_id);

    ret = do_query(query, &conn);
    if (ret != 0) {
        printf("执行查询失败：%s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    MYSQL_RES *result = mysql_use_result(conn);
    if (result == NULL) {
        printf("获取结果集失败：%s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    MYSQL_ROW row;
    if ((row = mysql_fetch_row(result)) == NULL) {
        printf("文件不存在\n");
        mysql_free_result(result);
        mysql_close(conn);
        return 1;
    }

    strcpy(md5_str, row[0]);
    *filesize = atoi(row[1]);

    mysql_free_result(result);
    mysql_close(conn);

    return 0;
}


// 初始化本地文件表
int init_local_file() {
    MYSQL *conn;
    int ret = mysqlconnect(&conn);
    if (ret != 0) {
        return EXIT_FAILURE;
    }

    ret = mysql_query(conn, "set names 'utf8'");
    if (ret != 0) {
        printf("error query1: %s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }

    char query[1024] = {0};

    // 读取目录流，获取本地存储目录中的每个文件
    DIR *dirp = opendir("NetDisk");
    if (!dirp) {
        perror("opendir");
        return EXIT_FAILURE;
    }

    struct dirent *pdirent;
    char md5_str[MD5_STR_LEN + 1] = {0};

    while ((pdirent = readdir(dirp)) != NULL) {
        // 跳过隐藏文件
        if (pdirent->d_name[0] == '.') {
            continue;
        }

        char real_name[100] = {0};
        strcpy(real_name, pdirent->d_name);
        sprintf(query, "select * from local_files where real_file_name like '%s'", real_name);
        ret = do_query(query, &conn);
        if (ret != 0) {
            return EXIT_FAILURE;
        }

        MYSQL_RES *result = mysql_store_result(conn);
        if (!result) {
            printf("error query: %s\n", mysql_error(conn));
            return EXIT_FAILURE;
        }

        int rows = mysql_num_rows(result);
        if (rows != 0) {
            mysql_free_result(result);
            continue;
        }
        mysql_free_result(result);

        char path[1024] = {0};
        sprintf(path, "%s/%s", "NetDisk", pdirent->d_name);
        bzero(md5_str, sizeof(md5_str));
        bzero(query, sizeof(query));
        sprintf(query, "insert into local_files(md5, link_num, real_file_name) values('%s', 0, '%s')", md5_str, real_name);
        ret = do_query(query, &conn);
        if (ret != 0) {
            return EXIT_FAILURE;
        }
    }

    mysql_close(conn);
    return 0;
}

// 增加虚拟文件
int add_virtual_file(int parent_id, char *filename, int owner_id, char *md5_str, int filesize)
{
    MYSQL *conn;
    int ret = mysqlconnect(&conn);
    if(ret != 0)
    {
        return EXIT_FAILURE;
    }
    ret = mysql_query(conn, "set names 'utf8'");
    if(ret != 0)
    {
        printf("error query1:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    char query[1024] = {0};
    sprintf(query, "insert into virtual_file(parent_id, filename, owner_id, md5, filesize, type) \
            values(%d, '%s', %d, '%s', %d, 'f')", parent_id, filename, owner_id, md5_str, filesize);
    ret = do_query(query, &conn);
    if(ret != 0)
    {
        return EXIT_FAILURE;
    }
    mysql_close(conn);
    return 0;
}

// 查找本地是否已经有文件存在
int if_local_exist(char *md5_str)
{
    MYSQL *conn;
    int ret = mysqlconnect(&conn);
    if(ret != 0)
    {
        return EXIT_FAILURE;
    }
    ret = mysql_query(conn, "set names 'utf8'");
    if(ret != 0)
    {
        printf("error query1:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    char query[1024] = {0};

    // 查询文件是否已在表中存在
    bzero(query, sizeof(query));
    sprintf(query, "select * from local_files where md5 like '%s'", md5_str);
    ret = do_query(query, &conn);
    if (ret != 0)
    {
        return EXIT_FAILURE;
    }
    MYSQL_RES *result = mysql_store_result(conn);// 这里如果用use不会真正获得数据，rows仍为0
    if (ret != 0)
    {
        printf("error query:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    int rows = mysql_num_rows(result);
    // printf("rows = %d\n", rows);
    if(rows == 0)// 文件不存在返回1
    {
        mysql_free_result(result);
        return 1;
    }
    mysql_free_result(result);
    mysql_close(conn);
    return 0;// 文件存在返回0
}


// 获取文件本地真实名称
int get_real_file_name(char *md5_str, char *real_file_name)
{
    MYSQL *conn;
    int ret = mysqlconnect(&conn);
    if(ret != 0)
    {
        return EXIT_FAILURE;
    }
    ret = mysql_query(conn, "set names 'utf8'");
    if(ret != 0)
    {
        printf("error query1:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    char query[1024] = {0};

    // 按照md5值在表中查找本地文件名
    bzero(query, sizeof(query));
    sprintf(query, "select real_file_name from local_files where md5 like '%s'", md5_str);
    ret = do_query(query, &conn);
    if (ret != 0)
    {
        return EXIT_FAILURE;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if(result == NULL)
    {
        printf("error result: %s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    int rows = mysql_num_rows(result);

    if(rows == 0)// 文件不存在返回1
    {
        mysql_free_result(result);
        mysql_close(conn);
        return 1;
    }
    MYSQL_ROW row;
    if((row = mysql_fetch_row(result)) == NULL)
    {
        printf("error query:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    strcpy(real_file_name, row[0]);
    mysql_free_result(result);
    mysql_close(conn);
    return 0;
}

// 增加本地文件的链接数
int add_link(char *md5_str)
{
    MYSQL *conn;
    int ret = mysqlconnect(&conn);
    int link_num;
    if(ret != 0)
    {
        return EXIT_FAILURE;
    }
    char query[1024] = {0};

    // 获得当前链接数
    bzero(query, sizeof(query));
    sprintf(query, "select link_num from local_files where md5 like '%s'", md5_str);
    ret = do_query(query, &conn);
    if (ret != 0)
    {
        return EXIT_FAILURE;
    }
    MYSQL_RES *result = mysql_use_result(conn);
    if (ret != 0)
    {
        printf("error query:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    MYSQL_ROW row;
    if((row = mysql_fetch_row(result)) == NULL)
    {
        printf("error query:%s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }
    link_num = atoi(row[0]);// 获得目前的链接数

    mysql_free_result(result);// 查询完要先释放查询结果，否则无法更新表格
    ++link_num;
    bzero(query, sizeof(query));
    sprintf(query, "update local_files set link_num = %d where md5 like '%s'", link_num, md5_str);
    // printf("query = %s\n", query);

    ret = do_query(query, &conn);
    if (ret != 0)
    {
        return EXIT_FAILURE;
    }

    mysql_close(conn);
    return 0;
}

// 在本地文件表中新增内容
int add_local_file(char *md5_str, char *real_name) {
    MYSQL *conn;
    int ret = mysqlconnect(&conn); // 连接数据库
    if (ret != 0) { // 连接失败，返回失败状态
        return EXIT_FAILURE;
    }
    ret = mysql_query(conn, "set names 'utf8'"); // 设置字符集
    if(ret != 0) {
        printf("error query1:%s\n", mysql_error(conn)); // 打印错误信息
        return EXIT_FAILURE;
    }
    char query[1024] = {0}; // 初始化查询语句字符串

    // 将 md5_str 和 real_name 插入到 local_files 表中
    bzero(query, sizeof(query)); // 清空查询字符串
    sprintf(query, "insert into local_files(md5, link_num, real_file_name) values('%s', 1, '%s')", md5_str, real_name); // 构建插入语句
    ret = do_query(query, &conn); // 执行插入查询
    if (ret != 0) { // 执行插入失败，返回失败状态
        return EXIT_FAILURE;
    }
    mysql_close(conn); // 关闭数据库连接
    return 0; // 返回成功状态
}

