//
// Created by 张亦乐 on 2018/6/21.
//

#include "WaitGroup.hpp"
using namespace std;
void WaitGroup::CountDown()
{
    lock_guard<mutex> lock(_mtx);
    _count--;
    if(_count == 0)
    {
        _cond.notify_all();
    }
}

unsigned long WaitGroup::GetCount()const
{
    return _count;
}

void WaitGroup::Wait()
{
    unique_lock<mutex> lock(_mtx);
    while(_count > 0)
    {
        _cond.wait(lock);
    }
}