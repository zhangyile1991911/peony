//
// Created by 张亦乐 on 2018/6/21.
//

#include <cstdio>
#include <functional>
#include "plog/Log.h"
#include "ReactorWorkerMgr.hpp"
#include "base/WaitGroup.hpp"
#include "ReactorWorker.hpp"
using namespace std;
ReactorWorkerMgr::ReactorWorkerMgr(Reactor& reactor,int workerNum):
        _nextIndex(0),
        _workerNum(workerNum),
        _started(false),
        _baseReactor(reactor)
{

}

ReactorWorkerMgr::~ReactorWorkerMgr()
{
    _started = false;
    for(unsigned long i = 0;i < _workers.size();i++)
    {
        auto p = _workers.back();
        delete p;
        _workers.pop_back();
    }
}

void ReactorWorkerMgr::Start()
{
    if(_started)return;


    if(_workerNum > 0)
    {
        WaitGroup waitGroup(_workerNum);
        for (unsigned long i = 0; i < _workerNum; ++i)
        {
            char buf[50];
            snprintf(buf,50,"worker_%ld",i);
            ReactorWorker* pWorker = new ReactorWorker(buf);
            pWorker->SetInitCallback([&](const string& name){
                waitGroup.CountDown();
                LOGD<<name<<" ReactorWorker inited";
            });

            _workers.push_back(pWorker);
            pWorker->StartWork();
        }
        //等待所有线程,初始化完成
        waitGroup.Wait();
        LOGD<<"all worker prepared";
    }
    _started = true;
}

Reactor* ReactorWorkerMgr::NextReactor()
{
    if(!_started)
    {
        return nullptr;
    }

    if(_workerNum == 0)
    {
        return &_baseReactor;
    }

    if(_nextIndex > _workers.size() - 1)
    {
        _nextIndex = 0;
    }

    return _workers[_nextIndex++]->GetReactor();
}