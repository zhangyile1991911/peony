//
// Created by 张亦乐 on 2018/6/21.
//


#ifdef __linux__
#include <sys/epoll.h>
#include <memory.h>
#include <cstdio>
#include <errno.h>
#include "Channel.hpp"
#include "EPoller.hpp"
EPoller::EPoller()
{
    _epollfd = epoll_create1(EPOLL_CLOEXEC);
    _eventList.resize(128);
}

EPoller::~EPoller()
{

}

int EPoller::Poll(int timeoutMs,ChannelList &activeChannel)
{
    int num = epoll_wait(_epollfd,&*_eventList.begin(),_eventList.size(), timeoutMs);

    if(num < 0)
    {
        printf("epoll_wait error %s\n",strerror(errno));
        return errno == EINTR ? 0 : num;
    }

    autoResize(num);

    activeChannel.clear();
    fillActiveChannel(activeChannel,num);

    return num;
}

void EPoller::AddChannel(Channel* channel)
{
    if(channel != nullptr)
    {
        struct epoll_event event;
        bzero(&event,sizeof(struct epoll_event));

        event.events = channel->GetEvent();
        event.data.ptr = channel;

        int fd = channel->GetFd();

        int retCode = epoll_ctl(_epollfd,EPOLL_CTL_ADD,fd,&event);
        if(retCode < 0)
        {
            //TODO 添加日志
            printf("epoll_ctl_add error fd = %d\n",fd);
        }
        else
        {
            //TODO 添加日志
        }
    }
}

void EPoller::DeleteChannel(Channel* channel)
{
    if(channel != nullptr)
    {
        struct epoll_event event;
        bzero(&event, sizeof(struct epoll_event));

        event.events = channel->GetEvent();
        event.data.ptr = channel;

        int fd = channel->GetFd();
        int retCode = epoll_ctl(_epollfd,EPOLL_CTL_DEL,fd,&event);
        if(retCode < 0)
        {
            //TODO 添加日志
            printf("epoll_ctl_mod error fd = %d\n",fd);
        }
        else
        {
            //TODO 添加日志
        }
    }
}

void EPoller::UpdateChannel(Channel* channel)
{
    if(channel != nullptr)
    {
        struct epoll_event event;
        bzero(&event, sizeof(struct epoll_event));

        event.events = channel->GetEvent();
        event.data.ptr = channel;

        int fd = channel->GetFd();
        int retCode = epoll_ctl(_epollfd,EPOLL_CTL_MOD,fd,&event);
        if(retCode < 0)
        {
            printf("epoll_ctl_mod error fd = %d\n",fd);
        }
    }
}

void EPoller::fillActiveChannel(ChannelList &activeChannel,int num)
{
    for (int i = 0; i < num; ++i)
    {
            Channel* c = (Channel*)_eventList[i].data.ptr;
            c->SetRevent(_eventList[i].events,0);
            activeChannel.push_back(c);
    }
}

void EPoller::autoResize(int num)
{
    //自动扩大
    if(num > 0 &&_eventList.size() == num)
    {
        _eventList.resize(_eventList.size()*2);
    }
}
#endif
