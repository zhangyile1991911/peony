//
// Created by 张亦乐 on 2018/6/21.
//

#ifndef POLLER_HPP
#define POLLER_HPP

#include <map>
#include <vector>

#include "base/Tools.hpp"
class Channel;
typedef std::map<int,Channel*> ChannelMap;//int = fd
typedef std::vector<Channel*>   ChannelList;
//Poller 对 kqueue epoll poll 抽象接口类
class Poller
{
    DISALLOW_COPY_AND_ASSIGN(Poller);
protected:
    bool hasChannel(Channel*);
    ChannelMap      _channelDict;
public:
    Poller(){}
    virtual ~Poller(){}
    //子类需要实现接口
    virtual int Poll(int timeoutMs,ChannelList& activeChannel) = 0;
    virtual void AddChannel(Channel*) = 0;
    virtual void DeleteChannel(Channel*) = 0;
    virtual void UpdateChannel(Channel*) = 0;
};

#endif //POLLER_HPP
