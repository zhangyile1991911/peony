//
// Created by 张亦乐 on 2018/6/28.
//

#ifndef TIMER_HPP
#define TIMER_HPP

#include <functional>
#include "net/Channel.hpp"
#include "net/Reactor.hpp"

class Timer
{
public:
    typedef std::function<void(Timer&)>   TimerCallback;
private:
    Channel         _channel;
    TimerCallback   _callback;
    bool            _isOnce;//在macos中实现 定时器调用一次后失效
private:
    int  handleReadCallback();
public:
    Timer(Reactor& reactor);
    ~Timer();
    void SetTimerCallback(TimerCallback cb){_callback = cb;}
    void Stop();
    void ResetInterval(int64_t microseconds);
    void ResetOnce(int64_t microseconds);
};
#endif //TIMER_HPP
