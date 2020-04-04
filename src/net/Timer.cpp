//
// Created by 张亦乐 on 2018/6/28.
// 适当地冗余代码 减少ifdef宏
#include <unistd.h>
#include <ctime>
#include <fcntl.h>
#include <sys/timerfd.h>

#include "net/Channel.hpp"
#include "net/Reactor.hpp"

#include "Timer.hpp"

int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0)
    {
        perror("timerfd_create");
    }
    return timerfd;
}
Timer::Timer(Reactor &reactor):
        _channel(reactor,createTimerfd())
{
    _channel.SetReadCallback(std::bind(&Timer::handleReadCallback,this));
    _channel.EnableRead();
}
int  Timer::handleReadCallback()
{
    uint64_t exp;
    read(_channel.GetFd(), &exp, sizeof(exp));
    if(_callback)_callback((*this));
	return OK;
}

void Timer::ResetInterval(int64_t microseconds)
{
    struct itimerspec value;
    value.it_value.tv_sec = microseconds/1000000;
    value.it_value.tv_nsec = (microseconds%1000000)*1000;
    value.it_interval.tv_sec = value.it_value.tv_sec;
    value.it_interval.tv_nsec = value.it_value.tv_nsec;

    if(timerfd_settime(_channel.GetFd(),0,&value,NULL) == -1)
    {
        perror("timerfd_settime()");
    }
}

void Timer::ResetOnce(int64_t microseconds)
{
    //秒->毫秒->微秒->纳秒
    //printf("Timer::ResetOnce %ld\n",microseconds);
    struct itimerspec value;
    value.it_value.tv_sec = microseconds/1000000;
    value.it_value.tv_nsec = (microseconds%1000000)*1000;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_nsec = 0;
    if(timerfd_settime(_channel.GetFd(),0,&value,NULL) < 0)
    {
        printf("Timer::ResetOnce microseconds = %ld next interval sec = %ld nsec = %ld \n",microseconds,value.it_value.tv_sec,value.it_value.tv_nsec);
        printf("timerfd_settime() error\n");
    }
}


Timer::~Timer()
{
    Stop();
}

void Timer::Stop()
{
    _channel.DisableRead();
}



