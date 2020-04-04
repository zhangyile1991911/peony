//
// Created by 张亦乐 on 2018/6/21.
//
#include <unistd.h>
#include "plog/Log.h"
#include "Channel.hpp"
#include "Reactor.hpp"
using namespace std;
atomic_int Channel::ChannelNum(0);
#ifdef __APPLE__
#include <sys/event.h>
#include <sys/socket.h>
Channel::Channel(Reactor& reactor,int fd,int custom_event):
        _reactor(reactor),
        _fd(fd),
        _join(false),
        _flags(EV_ADD),
        _custom_event(custom_event),
        _data(0),
        _fflags(0)
{
	Channel::ChannelNum.fetch_add(1);
	LOGD<<"now have "<< Channel::ChannelNum.load()<<" channel";
}

void Channel::SetRevent(int revent, uint16_t flag)
{
    //将计时事件转换成可读事件
    _revent = revent;
    _flag = flag;
}

void Channel::handleKevent()
{
    if(_errorCallback && (_flag & EV_ERROR))
    {
        _errorCallback();
        return;
    }
    if(_closeCallback && (_flag & EV_EOF))
    {
        _closeCallback();
        return;
    }
    if(_readCallback && (_revent == EVFILT_READ || _revent == EVFILT_TIMER || _revent == EVFILT_USER))
    {
        if(_readCallback && _readCallback() <= 0)return;
    }
    if(_writeCallback && (_revent == EVFILT_WRITE))
    {
        if(_writeCallback && _writeCallback() <= 0)return;
    }
}
void Channel::EnableRead(int64_t microsecond)
{
    _event = _custom_event;
    if(_event == EVFILT_READ)
    {
        _flags = EV_ADD;
    }
    if(_event == EVFILT_TIMER)
    {
        _data = microsecond;
        _fflags = NOTE_USECONDS;
        _flags = EV_ADD|EV_ENABLE;
    }
    update();
}

void Channel::DisableRead()
{
    _event = _custom_event;
    _flags = EV_DISABLE;
    update();
}

void Channel::EnableWrite()
{
    _event = WRITE_EVENT;
    _flags = EV_ADD|EV_ENABLE;
    update();
}

void Channel::DisableWrite()
{
    _event = WRITE_EVENT;
    _flags = EV_DISABLE;
    update();
}
void Channel::DisableAll()
{
    DisableRead();
    DisableWrite();
}

void Channel::FillKevent(struct kevent& evt)
{
    EV_SET(&evt,_fd,_event,_flags,_fflags,_data,(void*)this);
}

void Channel::TriggerUser()
{
    _event = EVFILT_USER;
    _flags = EV_ENABLE;
    _fflags = NOTE_FFCOPY|NOTE_TRIGGER|0x1;
    _data = 0;
    update();
}

void Channel::ResetTrigger()
{
    _event = EVFILT_USER;
    _flags = EV_DISABLE;
    _fflags = EV_CLEAR|NOTE_FFCOPY;
    _data = 0;
    update();
}

bool Channel::CanWrite()
{
    return _revent == EVFILT_WRITE;
}
#endif

#ifdef __linux__
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
#endif



Channel::~Channel()
{
	Channel::ChannelNum.fetch_sub(1);
    LOGD<<"Channel::~Channel() ::close() fd = "<<_fd;
	LOGD<<"now has "<<Channel::ChannelNum.load()<<" channels";
    ::close(_fd);
}

void Channel::HandleEvent()
{
#ifdef __APPLE__
    handleKevent();
#elif __linux__
    handleEpoll();
#endif
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
