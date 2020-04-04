//
// Created by 张亦乐 on 2018/6/22.
//
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <functional>
#include <cstring>//linux bzero

#include "plog/Log.h"
#include "Channel.hpp"
#include "Reactor.hpp"
#include "Acceptor.hpp"
#include "NetTools.hpp"

using namespace std;
Acceptor::Acceptor(Reactor& reactor,const char* ip, int port,bool reuse_addr,bool reuse_port):
        _reactor(reactor),
        _channelPtr(nullptr),
        _ip(ip),
        _port(port)
{

    int listenFD = create_socket();
    if(listenFD < 0)
    {
        exit(1);
    }

    struct sockaddr_in my_addr;
    bzero(&my_addr,sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    if(ip == nullptr)
    {
        my_addr.sin_addr.s_addr = inet_addr(ip);
    }
    else if(strcmp(ip,"*") == 0)
    {
        my_addr.sin_addr.s_addr = INADDR_ANY;
    }


    if(reuse_addr) set_reuse_addr(listenFD);

    if(reuse_port) set_reuse_port(listenFD);

    if(::bind(listenFD,(struct sockaddr*)&my_addr, sizeof(struct sockaddr)) < 0)
    {
        LOGE<<"bind() error";
        //perror("bind() error");
        exit(1);
    }
    if(::listen(listenFD,1024) < 0)
    {
        //perror("listen() error");
        LOGE<<"listen() error";
        exit(1);
    }

    if(set_noblock_fd(listenFD) < 0)
    {
        exit(1);
    }

    LOGD<<"Acceptor::Acceptor() listenfd ="<<listenFD;

    _channelPtr.reset(new Channel(_reactor,listenFD));
    _channelPtr->SetReadCallback(std::bind(&Acceptor::handleReadCallback,this));
    _channelPtr->EnableRead();
}

Acceptor::~Acceptor()
{}

int Acceptor::handleReadCallback()
{
    struct sockaddr_in remote_addr;
    socklen_t sin_size = sizeof(remote_addr);

//TODO 应该尝试多次调用accept,一次性读完所有新连接
#ifdef __linux__
    //将接受到的套接字,直接设置为非阻塞,只在linux内核高于3.53
    int newFD = ::accept4(_channelPtr->GetFd(),(struct sockaddr*)&remote_addr,&sin_size,SOCK_NONBLOCK);
#elif __APPLE__
    int newFD = accept(_channelPtr->GetFd(),(struct sockaddr*)&remote_addr,&sin_size);
    set_noblock_fd(newFD);
#endif

    if(newFD < 0)
    {
        //TODO 错误处理
        LOGD<<"Acceptor::handleReadCallback error = "<<errno;
		return FAIL;
    }
    else
    {
        if(_callback)
        {
            string  remote_ip(inet_ntoa(remote_addr.sin_addr));
            int     remote_port = ntohs(remote_addr.sin_port);
            _callback(newFD,remote_ip,remote_port);
        }
        else
        {
            LOGD<<"Acceptor::handleReadCallback 找不到处理新连接回调方法 关闭 fd = "<<newFD;
            close(newFD);
        }
    }
	return OK;
}
