//
// Created by 张亦乐 on 2018/6/21.
//

#include "base/WaitGroup.hpp"
#include "Worker.hpp"
#include "Reactor.hpp"
#include "ReactorWorker.hpp"

using namespace std;

ReactorWorker::ReactorWorker(char* name):_name(name),_worker(nullptr)
{
}

ReactorWorker::~ReactorWorker()
{
    if(_worker != nullptr)
    {
        delete _worker;
        _worker = nullptr;
    }
}

void ReactorWorker::doWork()
{
    //使用局部变量保证初始化顺序
    Reactor reactor;
    _reactor = &reactor;
    if(_initCallback != nullptr)
    {
        _initCallback(_name);
    }
    //Reactor进入死循环
    _reactor->Loop();
}

void ReactorWorker::StartWork()
{
    _worker = new Worker(_name);
    _worker->SetWorkCallback(bind(&ReactorWorker::doWork,this));
    _worker->StartWork();
}