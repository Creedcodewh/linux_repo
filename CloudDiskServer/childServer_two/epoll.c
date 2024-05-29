#include "head.h" 
int addEpollReadfd(int epfd, int fd)
{
    struct epoll_event evt;
    memset(&evt, 0, sizeof(evt));
    evt.data.fd = fd;
    evt.events = EPOLLIN;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &evt);
    ERROR_CHECK(ret, -1, "epoll_ctl");
    return 0;
}

int delEpollReadfd(int epfd, int fd)
{
    struct epoll_event evt;
    memset(&evt, 0, sizeof(evt));
    evt.data.fd = fd;
    evt.events = EPOLLIN;
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &evt);
    ERROR_CHECK(ret, -1, "epoll_ctl");
    return 0;
}
