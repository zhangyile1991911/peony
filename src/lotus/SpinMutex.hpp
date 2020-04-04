//
// Created by 张亦乐 on 2018/6/29.
//

#ifndef SPINMUTEX_HPP
#define SPINMUTEX_HPP

#include <atomic>
//TODO 根据系统实现不同的自旋锁
//linux http://guojing.me/linux-kernel-architecture/posts/spin-lock
class SpinMutex
{
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    SpinMutex(const SpinMutex&) = delete;
    SpinMutex& operator= (const SpinMutex&) = delete;
public:
    SpinMutex() = default;

    void lock()
    {
        while (flag.test_and_set(std::memory_order_acquire));
    }
    void unlock()
    {
        flag.clear(std::memory_order_release);
    }
};


#endif //SPINMUTEX_HPP
