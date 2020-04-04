//
// Created by 张亦乐 on 2018/6/21.
//

#ifndef EPOLLER_HPP
#define EPOLLER_HPP

#ifdef __linux__
#include <sys/epoll.h>

#include <vector>

#include "Poller.hpp"
class EPoller : public Poller
{
private:
    typedef std::vector<struct epoll_event>    EventList;
    int                 _epollfd;
    EventList           _eventList;
private:
    void    fillActiveChannel(ChannelList& activeChannel,int num);
    void    autoResize(int num);
public:
    EPoller();
    ~EPoller()override;

    int     Poll(int timeoutMs,ChannelList&  activeChannel)override;
    void    AddChannel(Channel*)override;
    void    DeleteChannel(Channel*)override;
    void    UpdateChannel(Channel*)override;
};
#endif

#endif //EPOLLER_HPP
