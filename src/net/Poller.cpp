//
// Created by 张亦乐 on 2018/6/21.
//
#include "Poller.hpp"
#include "Channel.hpp"
bool Poller::hasChannel(Channel* channel)
{
    if(channel == nullptr)return false;

    auto iter = _channelDict.find(channel->GetFd());
    return iter != _channelDict.end();
}