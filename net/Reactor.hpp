//
// Created by 张亦乐 on 2018/6/21.
//

#ifndef REACTOR_HPP
#define REACTOR_HPP

#include <thread>
#include <mutex>
#include <functional>
#include <vector>
#include <atomic>
#include <memory>

#include "base/Callback.hpp"
#include "base/Tools.hpp"
#include "base/Timestamp.hpp"
//向前声明
class Channel;
class Poller;
class TimerQueue;
class TimerTask;

//Reactor   只是收集,分发socket事件
//Reactor   不要继承!!!!
class Reactor
{
private:
    mutable std::atomic_bool             _quit;
    mutable std::atomic_bool             _taskHandling;
	mutable std::atomic_bool			 _polling;

    Poller*                      _poller;
    TimerQueue*                  _timerQueue;
    mutable std::mutex           _taskMutex;
    std::vector<Callback>        _taskList;

    const std::thread::id        _threadId;
    const int                    _timeMs;
	
	int							 _wakeupFD;	
	std::unique_ptr<Channel>	 _wakeupChannel;
private:
    void setQuit();
    void doTaskList();
	void wakeup();
    void addTimerTask(std::shared_ptr<TimerTask>);
    DISALLOW_COPY_AND_ASSIGN(Reactor)
public:
    Reactor(int timeMs = -1);
    ~Reactor();

    void Loop();
    void Quit();

    bool IsInCurrentThread();
    void AssertCurrentThread();
    void AddTask(Callback work);

    void RunAt(Callback,Timestamp);
    void RunAfter(Callback,int64_t microsec);//微秒
    void RunRepeated(Callback,int64_t microsec);//微秒

    void updateChannel(Channel*);
    void addChannel(Channel*);
    void delChannel(Channel*);

	const std::thread::id ThreadID()const{return _threadId;}
};

#endif //REACTOR_HPP
