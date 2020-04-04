//
// Created by 张亦乐 on 2018/6/21.
//
#include <stdio.h>
#include "plog/Log.h"
#include "Worker.hpp"

using namespace std;
atomic_int Worker::WorkerNum(0);
Worker::Worker(string workName) : _name(workName), _slave(nullptr)
{
	WorkerNum.fetch_add(1);
	LOGD<<"WorkerNum "<<WorkerNum.load();
}

Worker::~Worker()
{
    LOGD<<_name<<" Worker::~Worker()";
    if(_slave != nullptr)
    {
        _slave->join();
        delete _slave;
        _slave = nullptr;
    }
}

bool Worker::StartWork()
{
    _slave = new thread(bind(&Worker::doWork,this));
    return _slave != nullptr;
}

void Worker::doWork()
{
    if(_initCallback != nullptr)
    {
        _initCallback();
    }

    if(_workCallback != nullptr)
    {
        //printf("name = %s id = %d %d do work\n",_name.c_str(),_slave->get_id(),std::this_thread::get_id());
        _workCallback();
    }

    if(_finishCallback != nullptr)
    {
        _finishCallback();
    }
}
