//
// Created by 张亦乐 on 2018/6/21.
//
#ifdef __APPLE__

#include <errno.h>
#include <unistd.h>
#include "plog/Log.h"
#include "KQueuePoller.hpp"
#include "Channel.hpp"
KQueuePoller::KQueuePoller():_kqfd(kqueue())
{
    _eventList.resize(128);
}

KQueuePoller::~KQueuePoller()
{

}

int KQueuePoller::Poll(int timeoutMs,ChannelList& activeChannel)
{
    int num = 0;
    if(timeoutMs <= 0)
    {
        num = kevent(_kqfd, nullptr,0,&*_eventList.begin(),(int)_eventList.size(), nullptr);
    }
    else
    {//设置超时
        struct timespec ts;
        bzero(&ts, sizeof(struct timespec));
        ts.tv_sec = 0;
        ts.tv_nsec = timeoutMs*1000;
        num = kevent(_kqfd, nullptr,0,&*_eventList.begin(),(int)_eventList.size(), &ts);
    }

    if(num < 0)
    {
        LOGD<<"kevent errno = "<<strerror(errno);
        if(errno == EINTR)
        {
            LOGD<<"nothing happend but timeout";
            return 0;
        }
        return num;
    }

    autoResize(num);

    fillActiveChannel(activeChannel,num);

    return num;
}

void KQueuePoller::fillActiveChannel(ChannelList& activeChannel,int num)
{
    for (int i = 0; i < num; ++i)
    {
        Channel* c = (Channel*)_eventList[i].udata;
        c->SetRevent(_eventList[i].filter,_eventList[i].flags);
        activeChannel.push_back(c);
    }
}

void KQueuePoller::AddChannel(Channel* channel)
{
    if(channel == nullptr)return;

    struct kevent event{};
    channel->FillKevent(event);
    int result = kevent(_kqfd, &event, 1, nullptr, 0, nullptr);
    if(result < 0)
    {
        LOGD<<"AddChannel result < 0 fd = "<<event.ident<<" errno = "<<strerror(errno);
    }
}

void KQueuePoller::DeleteChannel(Channel* channel)
{
    if(channel == nullptr)return;

    struct kevent event{};
    EV_SET(&event,channel->GetFd(),channel->GetEvent(),EV_DELETE,0,0, (void*)channel);
    int result = kevent(_kqfd,&event,1, nullptr,0, nullptr);
    if(result < 0)
    {
        LOGD<<"DeleteChannel result < 0 fd = "<<event.ident<<" error = "<<strerror(errno);
    }
}

void KQueuePoller::UpdateChannel(Channel* channel)
{
    if(channel == nullptr)return;
    struct kevent evt{};
    channel->FillKevent(evt);
    int result = kevent(_kqfd,&evt,1, nullptr,0, nullptr);
    if(result < 0)
    {
        LOGD<<"UpdateChannel result < 0 fd = "<<evt.ident<<" error = "<<strerror(errno);
    }
}

void KQueuePoller::autoResize(unsigned long num)
{
    //自动扩大
    if(num > 0 && _eventList.size() == num)
    {
        _eventList.resize(_eventList.size()*2);
    }
}

#endif