//
// Created by 张亦乐 on 2018/6/22.
//
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <cstdio>
#include <cstring>

#include "NetTools.hpp"

int set_noblock_fd(int fd)
{
    int opts;
    opts = fcntl(fd,F_GETFL,0);
    if(opts < 0)
    {
        perror("fcntl GETFL error");
        return -1;
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(fd,F_SETFL,opts) < 0)
    {
        perror("fcntl SETFL error");
        return -1;
    }
    return 0;
}

int set_nodelay_fd(int fd)
{
    int on = 1;
    if(setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(void*)&on,sizeof(on)) < 0)
    {
        perror("set nodelay failed");
        return -1;
    }
    return 0;
}

int create_socket()
{
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
        perror("socket(AF_INET,SOCK_STREAM) error");
        return -1;
    }
    return sock;
}


#include <sys/epoll.h>
int create_epoll_fd()
{
    int epoll_fd;
    epoll_fd = epoll_create1(0);
    if(epoll_fd < 0)
    {
        perror("epoll_fd create failed");
        return -1;
    }
    return epoll_fd;
}


void set_reuse_addr(int sockfd)
{
    int optval = 1;
    int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval,socklen_t(sizeof(optval)));
    if(ret < 0)
    {
        perror("SO_REUSEADDR failed\n");
    }
}

void set_reuse_port(int sockfd)
{
    int optval = 1;
    int ret = ::setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,(const void *)&optval,socklen_t(sizeof(optval)));
    if(ret < 0)
    {
        perror("SO_REUSEPORT failed\n");
    }
}