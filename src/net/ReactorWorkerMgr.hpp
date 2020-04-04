//
// Created by 张亦乐 on 2018/6/21.
//

#ifndef REACTORWORKERMGR_HPP
#define REACTORWORKERMGR_HPP

#include <vector>
#include <memory>
#include <atomic>
//向前声明
class Reactor;
class ReactorWorker;

//可以设置为0个ReactorWorker
class ReactorWorkerMgr
{
private:
    unsigned long                       _nextIndex;
    unsigned long                       _workerNum;
    std::atomic_bool                    _started;
    //外部生命周期,Mgr不负责
    Reactor&                            _baseReactor;
    //线程池
    std::vector<ReactorWorker*>         _workers;
public:
    ReactorWorkerMgr(Reactor& reactor,int workerNum);
    ~ReactorWorkerMgr();

    void Start();
    Reactor* NextReactor() ;
};

#endif //REACTORWORKERMANAGER_HPP
