//
// Created by 张亦乐 on 2018/6/29.
//

#ifndef CONCURRENTQUEUE_HPP
#define CONCURRENTQUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include<cstdint>
//线程安全队列
template<typename T>
class ConcurrentQueue
{
private:
    mutable std::queue<T>           _queue;
    mutable std::mutex              _mutex;
    std::condition_variable         _cond;
public:
    // static std::atomic_uint64_t            PushNum;
	static std::atomic<std::uint64_t>		PushNum;
public:
    ConcurrentQueue() = default;
    ~ConcurrentQueue() = default;

    void    push(T q)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _queue.push(q);
            PushNum.fetch_add(1);
            //printf("ConcurrentQueue::push size = %lu\n",_queue.size());
        }
        _cond.notify_one();
    }

    T    front()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while(_queue.empty())
        {
            //printf("ConcurrentQueue::front wait\n");
            _cond.wait(lock);
        }
        T t = _queue.front();
        _queue.pop();
        //printf("ConcurrentQueue::front size = %lu\n",_queue.size());
        return t;
    }

    bool   empty()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.empty();
    }
};
template<typename T>
std::atomic<std::uint64_t> ConcurrentQueue<T>::PushNum(0);
#endif //CONCURRENTQUEUE_HPP
