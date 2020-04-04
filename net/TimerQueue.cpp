//
// Created by 张亦乐 on 2018/7/30.
//
#include "net/Reactor.hpp"
#include "net/Timer.hpp"

#include "TimerQueue.hpp"
using namespace std;
TimerQueue::TimerQueue(Reactor& reactor):
        _reactor(reactor),
        _timer(reactor),
        _earliest(-1),
        _handleExpiration(false)
{
    _timer.SetTimerCallback(std::bind(&TimerQueue::handleTimeouts,this));
}

void TimerQueue::handleTimeouts()
{
    TimerTaskList reactiveTask;
    Timestamp now = Timestamp::Now();
    _handleExpiration = true;
    auto iter = _taskDict.begin();
    while(iter != _taskDict.end())
    {
        shared_ptr<TimerTask> task = iter->second;
        bool isExpired = task->isExpired(now);
        //printf("id = %ld isExpired = %d\n",task->ID(),isExpired);
        if(isExpired)
        {
            task->Run();
            if(task->isRepeated())
            {
                task->Restart();
                reactiveTask.push_back(task);
            }
            iter = _taskDict.erase(iter);
        }
        else
    {
        ++iter;
    }
}

    addTimerTask(reactiveTask);
    if(!_taskDict.empty())
    {
        resetTimer();
    }
    _handleExpiration = false;
}

void TimerQueue::AddTimerTask(Callback cb,Timestamp when, int64_t interval)
{
    auto taskPtr = shared_ptr<TimerTask>(new TimerTask(cb,when,interval));
    _taskDict[taskPtr->Expiration()] = taskPtr;
    resetTimer();
}

void TimerQueue::AddTimerTask(std::shared_ptr<TimerTask> taskPtr)
{
    if(!_reactor.IsInCurrentThread())
    {
        _reactor.AddTask([this,taskPtr](){
            _taskDict[taskPtr->Expiration()] = taskPtr;
            resetTimer();
        });
    }
    else if(_handleExpiration)
    {
        _reactor.AddTask([this,taskPtr](){
            _taskDict[taskPtr->Expiration()] = taskPtr;
            resetTimer();
        });
    }
    else
    {
        _taskDict[taskPtr->Expiration()] = taskPtr;
        resetTimer();
    }
}
void TimerQueue::addTimerTask(TimerTaskList& taskList)
{
    auto iter = taskList.begin();
    for(;iter != taskList.end();++iter)
    {
        auto task = (*iter);
        _taskDict[task->Expiration()] = task;
    }
}

void TimerQueue::resetTimer()
{
    auto timestamp = _taskDict.begin()->first;
    auto taskPtr = _taskDict.begin()->second;
    if(_earliest == -1 || timestamp.Epoch() != _earliest)
    {
        int64_t nextTimeout = Timestamp::NowTimeDiff(timestamp);
        _timer.ResetOnce(nextTimeout);
    }
};