//
// Created by 张亦乐 on 2018/6/21.
//

#ifndef ATELIER_WAITGROUP_HPP
#define ATELIER_WAITGROUP_HPP
#include <mutex>
#include <condition_variable>

class WaitGroup
{
public:
    explicit WaitGroup(unsigned long num):_count(num){}
    ~WaitGroup(){}

    void              CountDown();
    unsigned long     GetCount()const;
    void              Wait();
private:
    unsigned long               _count;
    mutable std::mutex          _mtx;
    std::condition_variable     _cond;
};
#endif //ATELIER_WAITGROUP_HPP
