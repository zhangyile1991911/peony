//
// Created by zhangyile on 18-7-13.
//

#ifndef PEONY_TIMERTASK_HPP
#define PEONY_TIMERTASK_HPP

#include <atomic>

#include "base/Timestamp.hpp"
#include "base/Callback.hpp"
//warning:不可以做非常耗时的任务
class TimerTask
{
private:
    Timestamp   _expiration;
    int64_t    _intervalms;//微妙
    bool       _repeated;
    Callback   _callback;
    int64_t    _taskID;
public:
    //param: 回调,超时时间戳(微妙),间隔触发(微妙)
    TimerTask(Callback callback,Timestamp when,int64_t intervalMS):
            _expiration(when),
            _intervalms(intervalMS),
            _repeated(intervalMS != 0),
            _callback(std::move(callback)),
            _taskID(TaskNum.fetch_add(1))
    {
        //printf("TimerTask constructor id = %ld repeated %d\n",_taskID,_repeated);
    }
    ~TimerTask()
    {
        //printf("TimerTask destructor id = %ld repeated %d\n",_taskID,_repeated);
    }
    int64_t ID(){return _taskID;}
    void Run()
    {
        _callback();
    }
    void Restart()
    {
        _expiration = Timestamp::AddCurrentTime(_intervalms);
    }
    bool isRepeated(){return _repeated;}
    bool isExpired(const Timestamp& ts){return ts >= _expiration;}
    const Timestamp& Expiration()const {return _expiration;}
    static std::atomic_uint_least64_t TaskNum;
};

#endif //PEONY_TIMERTASK_HPP
