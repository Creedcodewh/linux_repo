#include "head.h"

int cdCommand(task_t* task){
    int user_id = task->pclientQueue->client[task->peerfd];//从已经连接的客户端队列中获取该用户的userid
    char  dir_name[100] ={0} ;//获取参数

    int code;//当前目录文件的id
    int paraent_code;//父id
    get_user_code(user_id,&code);//获取用户所在目录的编号
    paraent_code = code;
    char pwd[1024] = {0};//用来存储当前目录
    get_user_pwd(user_id,pwd);//获取用户当前所在目录
    char *path_words[20] = {0};//用来存储分割路径参数的字符串数组
    int pathWordsNUm = 0;//路径参数分割个数
    char s[100] = {0};//暂时存储

    //1、当我直传了CD命令，没有参数，此时返回家目录
    if(task->cmdLength == task->preLength){//总长度为命令长度，说明没有参数。此时直接回到家目录
                                           //直接返回到家目录
        paraent_code = code = 0;
        strcpy(pwd,"~/");
        update_user_pwd(user_id, pwd, code);
        sendMessage(task->peerfd,pwd);
        return 0;
    }

    //2、有参数，即有路径

    //2.1 路径第一符号是/，此时不合法。返回路径不合法
    strncpy(dir_name,task->data + task->preLength,task->cmdLength - task->preLength);//已经获取了路径名
    if(dir_name[0] == '/'){
        sendMessage(task->peerfd,"路径不合法");
        return -1;
    }


    //2.2我的命令只有..或者../ 此时返回上一级目录
    if(strcmp(dir_name, "..") == 0 || strcmp(dir_name, "../") == 0){

        //如果当前所在目录编号为0，说明已经在家目录了,不能返回上一级
        if(code == 0){
            sendMessage(task->peerfd,"已在根目录,非法操作");
            return -1;
        }

        //不在根目录，就要获取父亲节点,后面的操作就是把最后一个文件名去掉，然后就成了当前工作目录
        get_parent_id(code,&paraent_code);
        code = paraent_code;//把当前目录id改为父亲节点的id
        strcpy(s,pwd);//把当前目录切分
        path_words[pathWordsNUm] = strtok(s,"/");
        ++pathWordsNUm;
        char *token = strtok(NULL, "/");
        while (token != NULL) {
            path_words[pathWordsNUm] = token;
            ++pathWordsNUm;
            token = strtok(NULL, "/");
        }//已经全部分割，并保存起来
        bzero(pwd, sizeof(pwd));
        for(int j = 0 ;j < pathWordsNUm -1 ;++j){//目的就是把最后一个路径名去掉，达到返回上一层的目的
            strcat(pwd, path_words[j]);
            strcat(pwd, "/");
        }
    }

    //2.3 我的命令只有~/和~，其效果和cd不带参数一样，直接返回到家目录
    else if(strcmp(dir_name, "~/") == 0 || strcmp(dir_name, "~") == 0){// 回到根目录
        strcpy(pwd,"~/");//根目录
        code = 0;//根目录的编号为0
    }

    //2.4 现在就是正常的参数，但是参数里面也包含着那些特殊符号，把路径切分，对每一个切割字符处理
    else{//去往目标地址
        bzero(s, sizeof(s));
        bzero(path_words, sizeof(path_words));
        strcpy(s, dir_name);//把目的路径复制给s，分割s
        path_words[pathWordsNUm] = strtok(s,"/");//有可能只有一个参数 后面不带/,待优化
        ++pathWordsNUm;
        while ((path_words[pathWordsNUm] = strtok(NULL, "/")) != NULL)
        {
            ++pathWordsNUm;
        }//已经全部切割好并保存在路径字符串数组里面

        //对每一个路径字符进行处理
        for(int i = 0;i < pathWordsNUm;++i){

            // 遇到"."符号，忽略
            if(strcmp(path_words[i],".") == 0){//当前的路径字符是. ,相当于不执行操作
                continue;
            }

            //遇到"~"符号,回到家目录
            else if(strcmp(path_words[i],"~") == 0){//回到家目录
                paraent_code = code = 0;
                continue;
            }

            //遇到".."符号，要退一级。如果已经在家目录了，就是非法。如果没在，就获取父节点编号，更新code
            else if(strcmp(path_words[i], "..") == 0){//要往上退一级
                if(code == 0){//已经在根目录，并不能往上退
                              //路径不合法
                    sendMessage(task->peerfd,"路径不合法");
                    return 0;
                }else{
                    get_parent_id(code,&paraent_code);//获取上一级的目录的代号
                    code = paraent_code;//把当前目录代号改为上一级的，相当于虚拟执行了返回上一级
                    continue;
                }
            }

            //不是三个特殊的符号 "."" ".." "~",正常路径名
            else{
                char type[2];
                int ret = get_file_code(user_id, path_words[i], &code, paraent_code, type);
                if(ret !=0 || strcmp(type,"d") != 0 ){// 文件查找失败；不是目录文件；不在可搜索路径下
                    sendMessage(task->peerfd,"路径不是一个目录文件，非法操作\n");
                    return -1;
                }else{//这里有点不太清晰
                    if(paraent_code !=0){
                        strcat(pwd,"/");//在当前目录上拼接上一个/
                    }
                    strcat(pwd,path_words[i]);
                    paraent_code = code;
                }
            }
        }//执行完毕
    }
    printf("pwd = %s, code = %d\n", pwd, code);
    update_user_pwd(user_id, pwd, code);
    sendMessage(task->peerfd,pwd);
    return 0;
}

int lsCommand(task_t * task){
    //查看当前目录
    //首先获取当前目录
    int user_id = task->pclientQueue->client[task->peerfd];
    int code;//目录id
    int paraent_code;//父节点
    get_user_code(user_id, &code);
    paraent_code = code;
    char  dir_name[100] ={0} ;//存参数

    printf("test：当前目录的id%d\n",code);
    //1、没有参数,"." "./"都在当前路径，不用改变code
    if(task->cmdLength == task->preLength){
        printf("不切换\n");
    }else{

        //2、有参数
        strncpy(dir_name,task->data + task->preLength, task->cmdLength - task->preLength);
        if(dir_name[0] == '/'){
            sendMessage(task->peerfd,"路径不合法");
            return -1;
        }
        if(strcmp(dir_name,".") == 0 || strcmp(dir_name,"./") == 0){
            printf("不切换\n");
        }

        else{
            char pwd[1024] = {0};//用来存储当前目录
            get_user_pwd(user_id, pwd);
            char *path_words[20] = {0};//用来存储分割路径参数的字符串数组
            int pathWordsNUm = 0;//路径参数分割个数
            char s[100] = {0};//暂时存储
                              //2.2我的命令只有..或者../ 此时返回上一级目录
            if(strcmp(dir_name, "..") == 0 || strcmp(dir_name, "../") == 0){

                //如果当前所在目录编号为0，说明已经在家目录了,不能返回上一级
                if(code == 0){
                    sendMessage(task->peerfd,"已在根目录,非法操作");
                    return -1;
                }                                                                                                                             

                //不在根目录，就要获取父亲节点,后面的操作就是把最后一个文件名去掉，然后就成了当前工作目录
                get_parent_id(code,&paraent_code);
                code = paraent_code;//把当前目录id改为父亲节点的id
            }
            //不是..就去往目的地址
            else{//去往目标地址
                bzero(s, sizeof(s));
                bzero(path_words, sizeof(path_words));
                strcpy(s, dir_name);//把目的路径复制给s，分割s
                path_words[pathWordsNUm] = strtok(s,"/");//有可能只有一个参数 后面不带/,待优化
                ++pathWordsNUm;
                while ((path_words[pathWordsNUm] = strtok(NULL, "/")) != NULL)
                {
                    ++pathWordsNUm;
                }//已经全部切割好并保存在路径字符串数组里面

                //对每一个路径字符进行处理
                for(int i = 0;i < pathWordsNUm;++i){

                    // 遇到"."符号，忽略
                    if(strcmp(path_words[i],".") == 0){//当前的路径字符是. ,相当于不执行操作
                        continue;
                    }

                    //遇到"~"符号,回到家目录
                    else if(strcmp(path_words[i],"~") == 0){//回到家目录
                        paraent_code = code = 0;
                        continue;
                    }

                    //遇到".."符号，要退一级。如果已经在家目录了，就是非法。如果没在，就获取父节
                    else if(strcmp(path_words[i], "..") == 0){//要往上退一级
                        if(code == 0){//已经在根目录，并不能往上退
                                      //路径不合法
                            sendMessage(task->peerfd,"路径不合法");
                            return 0;
                        }else{
                            get_parent_id(code,&paraent_code);//获取上一级的目录的代号
                            code = paraent_code;//把当前目录代号改为上一级的，相当于虚拟执行了返
                            continue;
                        }
                    }
                    //不是三个特殊的符号 "."" ".." "~",正常路径名
                    else{
                        char type[2];
                        int ret = get_file_code(user_id, path_words[i], &code, paraent_code, type);
                        if(ret !=0 || strcmp(type,"d") != 0 ){// 文件查找失败；不是目录文件；不在可搜索路径下
                            sendMessage(task->peerfd,"路径不是一个目录文件，非法操作\n");
                        }
                    }
                    paraent_code = code;
                }
            }
        }
    }
    //test 
    printf("目标路径的id为%d\n",code);
    sendMessage(task->peerfd,"路径存在");
    find_code_files(task->peerfd, code, user_id);// 把找到的文件发给客户端
    return 0;
}

int mkdirCommand(task_t * task){
    int code;
    int user_id = task->pclientQueue->client[task->peerfd];
    get_user_code(user_id, &code);

    //1、先把文件名参数提取出来
    char dir_name[50] = {};
    if(task->cmdLength == task->preLength){//没有参数
        sendMessage(task->peerfd,"请输入文件夹名字");
        return -1;
    }
    strncpy(dir_name,task->data + task->preLength,task->cmdLength-task->preLength);

    //2、检查该文件名字在数据库中是否已经存在
    //   //待优化找出来的文件名要一样，然后父节点和当前code要一样，且所属于者也要一样 
    char type[2] = {0};
    int paraent_code = 0;
    int ret = get_file_code(user_id, dir_name, &paraent_code, code, type);//查询成功的话，说明该名称的文件夹已经存在。
    if(ret == 0){
        sendMessage(task->peerfd,"输入文件名重复");
        return -1;
    }

    //3、不存在则添加虚拟文件夹
    ret = add_virtual_dir(code,dir_name,user_id);//code作为新文件夹的父id，输入的参数作为文件名
    if(ret != 0){
        sendMessage(task->peerfd,"创建失败");
        return -1;
    } 

    sendMessage(task->peerfd,"创建成功");
    return 0;
}

int pwdCommand(task_t * task){

    int user_id = task->pclientQueue->client[task->peerfd];
    char pwd[1024] = "";
    get_user_pwd(user_id,pwd);
    sendMessage(task->peerfd,pwd);
    return 0;
}

int removeCommand(task_t * task){
    int user_id = task->pclientQueue->client[task->peerfd];
    char file_name[100] = "";
    int code;
    int paraent_code;
    char type[2] = "";
    char file[100] = "";

    //1、没有参数，返回错误
    if(task->cmdLength == task->preLength){
        sendMessage(task->peerfd,"请输入你要删除的文件名");
        return -1;
    }
    strncpy(file_name,task->data + task->preLength,task->cmdLength - task->preLength);
    get_user_code(user_id,&code);
    paraent_code = code;
    if(file_name[0] == '/' \
       || strcmp(file_name, ".") == 0 
       || strcmp(file_name, "~/") == 0 
       || strcmp(file_name, "./") == 0 
       || strcmp(file_name, "..") == 0 
       || strcmp(file_name, "../") == 0){

        sendMessage(task->peerfd,"路径不合法");
    }else{
        char *path_words[20] = {0};
        int pathWordsNUm = 0;
        char s[100] = {0};
        strcpy(s,file_name);
        path_words[pathWordsNUm] = strtok(s,"/");
        ++pathWordsNUm;
        char *token = strtok(NULL, "/");
        while (token != NULL) {
            path_words[pathWordsNUm] = token;
            ++pathWordsNUm;
            token = strtok(NULL, "/");
        }//已经全部分割，并保存起来

        for(int i = 0;i < pathWordsNUm;++i){
                // 遇到"."符号，忽略
               if(strcmp(path_words[i],".") == 0){//当前的路径字符是. ,相当于不执行操作
                   continue;
               }
               
               //遇到"~"符号,回到家目录
               else if(strcmp(path_words[i],"~") == 0){//回到家目录
                   paraent_code = code = 0;
                   continue;
               }
   
               //遇到".."符号，要退一级。如果已经在家目录了，就是非法。如果没在，就获取父节点编号，>
               else if(strcmp(path_words[i], "..") == 0){//要往上退一级
                   if(code == 0){//已经在根目录，并不能往上退
                                 //路径不合法
                       sendMessage(task->peerfd,"路径不合法");
                       return 0;
                   }else{
                       get_parent_id(code,&paraent_code);//获取上一级的目录的代号
                       code = paraent_code;//把当前目录代号改为上一级的，相当于虚拟执行了返回上一级
                       continue;
                   }
               }
               int ret = get_file_code(user_id,path_words[i],&code,paraent_code,type);
               if(ret != 0){
                   sendMessage(task->peerfd,"路径不存在");
                    return -1;
               }
               if(i != pathWordsNUm -1){
                   paraent_code = code;//进入下一级
               }
        }
        strcpy(file,path_words[pathWordsNUm-1]);//最后一个是目标删除文件
    }
    if(code == 0){
        sendMessage(task->peerfd,"当前目标文件属于家目录，您想当没家的孩子吗？");
        return -1;
    }

    if(strcmp(type,"f") == 0){
        printf("文件删除还没写呢\n");
        return -1;
    }
    del_virtual_file(code, user_id);//首先要删掉指定文件，以及文件下面一级的文件夹
    sendMessage(task->peerfd,"删除成功");
    return 0;
}
