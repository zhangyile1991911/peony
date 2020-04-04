//
// Created by 张亦乐 on 2018/6/21.
//

#ifndef KQUEUEPOLLER_HPP
#define KQUEUEPOLLER_HPP


#ifdef __APPLE__
#include <sys/event.h>
#include <sys/types.h>

#include <vector>

#include "Poller.hpp"

class KQueuePoller : public Poller
{
private:
    typedef std::vector<struct kevent>   EventList;
    int                             _kqfd;
    EventList                       _eventList;
private:
    void    fillActiveChannel(ChannelList& activeChannel,int num);
    void    autoResize(unsigned long num);
public:
    KQueuePoller();
    ~KQueuePoller();
    //实现父接口方法
    int     Poll(int timeoutMs,ChannelList& activeChannel)override;
    void    AddChannel(Channel*)override;
    void    DeleteChannel(Channel*)override;
    void    UpdateChannel(Channel*)override;
};
#endif

#endif //KQUEUEPOLLER_HPP
