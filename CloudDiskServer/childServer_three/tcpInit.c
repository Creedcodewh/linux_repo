#include "head.h"

int tcpInit(int *psockfd, char *file){
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }

    char ip[1024] = {0};
    char port[1024] = {0};
    char buf[1024] = {0};

    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fclose(fp);
        fprintf(stderr, "Error reading IP from file\n");
        return -1;
    }

    int i = 0;
    while (buf[i] < '0' || buf[i] > '9') {
        ++i;
    }
    strcpy(ip, buf + i);

    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fclose(fp);
        fprintf(stderr, "Error reading port from file\n");
        return -1;
    }

    i = 0;
    while (buf[i] < '0' || buf[i] > '9') {
        ++i;
    }
    strcpy(port, buf + i);

    fclose(fp);
    *psockfd = socket(AF_INET,SOCK_STREAM,0);
    int flag = 1;
    setsockopt(*psockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));
    addr.sin_addr.s_addr = inet_addr(ip);
    bind(*psockfd,(struct sockaddr *)&addr,sizeof(addr));
    listen(*psockfd,10);
    return 0;

}
