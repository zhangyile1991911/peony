//
// Created by 张亦乐 on 2018/6/21.
//
#include <unistd.h>
#include "plog/Log.h"
#include "Channel.hpp"
#include "Reactor.hpp"
using namespace std;
atomic_int Channel::ChannelNum(0);

Channel::Channel(Reactor& reactor,int fd):
        _reactor(reactor),
        _fd(fd),
        _join(false)
{
    Channel::ChannelNum.fetch_add(1);
    LOGD<<"now have "<< Channel::ChannelNum.load()<<" channel";
}
void Channel::SetRevent(int revent, uint16_t flag)
{
    _revent = revent;
}

void Channel::handleEpoll()
{
    if ((_revent & EPOLLHUP) || (_revent & EPOLLRDHUP))
    {
        if (_closeCallback) _closeCallback();
		return;
    }
    if (_revent & EPOLLERR)
    {
        if (_errorCallback)_errorCallback();
		return;
    }
    if (_revent & (EPOLLIN | EPOLLPRI))
    {
        if (_readCallback && _readCallback() <= 0)return;	
    }
    if (_revent & EPOLLOUT)
    {
        if (_writeCallback && _writeCallback() <= 0)return;
    }
}
void Channel::EnableRead(int64_t ms)
{
    _event |= READ_EVENT;
    update();
}

void Channel::EnableWrite()
{
    _event |= WRITE_EVENT;
    update();
}

void Channel::DisableWrite()
{
    _event &= ~WRITE_EVENT;
    update();
}

void Channel::DisableRead()
{
    _event &= ~READ_EVENT;
    update();
}
void Channel::DisableAll()
{
    _event = NONE_EVENT;
    update();
}

bool Channel::CanWrite()
{
//    bool write = false;
//
//    write = _revent & EPOLLOUT;

    return _revent & EPOLLOUT;
}



Channel::~Channel()
{
	Channel::ChannelNum.fetch_sub(1);
    LOGD<<"Channel::~Channel() ::close() fd = "<<_fd;
	LOGD<<"now has "<<Channel::ChannelNum.load()<<" channels";
    ::close(_fd);
}

void Channel::HandleEvent()
{
    handleEpoll();
}

void Channel::update()
{
    if(!_join)
    {
        _reactor.addChannel(this);
        _join = true;
    }
    else
    {

        _reactor.updateChannel(this);
    }
}
