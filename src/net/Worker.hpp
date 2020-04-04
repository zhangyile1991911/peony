//
// Created by 张亦乐 on 2018/6/21.
//

#ifndef WORKER_HPP
#define WORKER_HPP

#include <mutex>
#include <thread>
#include <string>
#include <atomic>
#include <vector>
#include <functional>

#include "base/Callback.hpp"
#include "base/Tools.hpp"

class Worker
{
private:
    void doWork();

private:
    const std::string  _name;
    mutable std::mutex _lock;
    std::thread*       _slave;
	static std::atomic_int WorkerNum;
private:
    Callback _initCallback;
    Callback _workCallback;
    Callback _finishCallback;
    DISALLOW_COPY_AND_ASSIGN(Worker)
public:
    explicit Worker(std::string workName);
    ~Worker();

    const std::string& Name() const
    {
        return _name;
    }

    bool StartWork();

    void SetInitCallback(Callback cb) { _initCallback = std::move(cb); }
    void SetWorkCallback(Callback cb) { _workCallback = std::move(cb); }
    void SetFinishCallback(Callback cb) { _finishCallback = std::move(cb); }
};

#endif //WORKER_HPP
