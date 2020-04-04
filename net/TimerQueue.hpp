//
// Created by 张亦乐 on 2018/7/30.
//

#ifndef PEONY_TIMERQUEUE_HPP
#define PEONY_TIMERQUEUE_HPP

#include <atomic>
#include <mutex>
#include <map>
#include <memory>


#include "net/TimerTask.hpp"
#include "net/Timer.hpp"
typedef std::map<Timestamp,std::shared_ptr<TimerTask>> TimerTaskDict;
typedef std::vector<std::shared_ptr<TimerTask>>  TimerTaskList;
class TimerQueue
{
private:
    Reactor&                _reactor;
    TimerTaskDict           _taskDict;
    Timer                   _timer;
    int64_t                 _earliest;
    std::atomic_bool        _handleExpiration;
    std::mutex              _mutex;
private:
    void handleTimeouts();
    void addTimerTask(TimerTaskList& taskList);
    void resetTimer();
public:
    TimerQueue(Reactor& reactor);
    ~TimerQueue() = default;
    void AddTimerTask(Callback cb,Timestamp when, int64_t interval);
    void AddTimerTask(std::shared_ptr<TimerTask>);
};


#endif //PEONY_TIMERQUEUE_HPP
