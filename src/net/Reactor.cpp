//
// Created by 张亦乐 on 2018/6/21.
//


#include <thread>
#include <cassert>

#include <sys/eventfd.h>
#include <string.h>
#include "EPoller.hpp"

#include "Reactor.hpp"
#include "Channel.hpp"
#include "TimerQueue.hpp"
#include "plog/Log.h"

int createEventFD()
{
	int evtfd = ::eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
	if(evtfd < 0)
	{
		LOGD<<"failed in createEventFD";
		exit(-1);
	}
	LOGD<<"create event fd = "<<evtfd;
	return evtfd;
}

using namespace std;
Reactor::Reactor(int timeMs):
        _quit(false),
        _taskHandling(false),
		_polling(false),
        _poller(nullptr),
        _timerQueue(nullptr),
        _threadId(std::this_thread::get_id()),
        _timeMs(timeMs)
{
    //printf("Reactor::Reactor in thread %ld\n",_threadId);
    _poller = new EPoller();
    _wakeupFD = createEventFD();
    _wakeupChannel.reset(new Channel(*this,_wakeupFD));
    _wakeupChannel->SetReadCallback([this]()->int
    {
        uint64_t wakeupSignal;
        ssize_t n = ::read(_wakeupFD,&wakeupSignal,sizeof(wakeupSignal));
        if(n != sizeof(wakeupSignal))
        {
            LOGD<<"error in read WakeupChannel";
        }
        return 0;
    });
}

Reactor::~Reactor()
{
    LOGD<<"Reactor::~Reactor()";
    if(_poller != nullptr)
    {
        delete _poller;
        _poller = nullptr;
    }
}

void Reactor::Loop()
{
    int result;
    vector<Channel*> activeChannelList;
    LOGD<<"start Reactor::Loop()";
	_wakeupChannel->EnableRead();
    while (!_quit)
    {
        activeChannelList.clear();
        result = _poller->Poll(_timeMs,activeChannelList);
        if(result < 0)break;

		_polling = true;	
        for(auto c : activeChannelList)
        {
            c->HandleEvent();
        }
		_polling = false;	
        doTaskList();
    }
    LOGD<<"finish Reactor::Loop()";
}

void Reactor::Quit()
{
    if(!_taskHandling)
    {
        AddTask(std::bind(&Reactor::setQuit,this));
    }
    else
    {
        setQuit();
    }
}

void Reactor::setQuit()
{
    _quit = true;
}
void Reactor::doTaskList()
{
    vector<Callback> tmp;
    {
        lock_guard<mutex>   autolock(_taskMutex);
		tmp.swap(_taskList);
    }
	_taskHandling = true;
	for(auto& f : tmp)
	{
		f();
	}
	_taskHandling = false;
}

void Reactor::wakeup()
{
	uint64_t wakeSignal = 1;
	ssize_t n = ::write(_wakeupFD,&wakeSignal,sizeof(wakeSignal));
	if(n != sizeof(wakeSignal))
	{
		printf("error in Reactor::wakeup() errno = %d str = %s\n",errno,strerror(errno));
	}

}

void Reactor::AddTask(Callback task)
{
    if(IsInCurrentThread())
    {
        task();
    }
    else
    {
		{
			lock_guard<mutex>  autoLock(_taskMutex);
			_taskList.push_back(task);
		}
		if(!_polling || _taskHandling)
		{
			wakeup();
		}
    }
}


void Reactor::updateChannel(Channel* channel)
{
    AssertCurrentThread();
    if(channel != nullptr)
    {
        _poller->UpdateChannel(channel);
    }
}

void Reactor::addChannel(Channel* channel)
{
    AssertCurrentThread();
    if(channel != nullptr)
    {
        _poller->AddChannel(channel);
    }
}

void Reactor::delChannel(Channel* channel)
{
    AssertCurrentThread();
    if(channel != nullptr)
    {
        _poller->DeleteChannel(channel);
    }
}

bool Reactor::IsInCurrentThread()
{
    return this_thread::get_id() == _threadId;
}
void Reactor::AssertCurrentThread()
{
    if(!IsInCurrentThread())
    {
        printf("AssertCurrentThread reactor = current = \n");
    }
}

void Reactor::addTimerTask(shared_ptr<TimerTask> taskPtr)
{
    if(!taskPtr.use_count())
    {
        return;
    }
    if(_timerQueue == nullptr)
    {
        _timerQueue = new TimerQueue(*this);
    }
    _timerQueue->AddTimerTask(taskPtr);
}

void Reactor::RunAt(Callback cb,Timestamp ts)
{
    addTimerTask(shared_ptr<TimerTask>(new TimerTask(cb,ts,0)));
}

void Reactor::RunAfter(Callback cb,int64_t microsec)
{
    addTimerTask(shared_ptr<TimerTask>(new TimerTask(cb,Timestamp::AddCurrentTime(microsec),false)));
}

void Reactor::RunRepeated(Callback cb,int64_t microsec)
{
    addTimerTask(shared_ptr<TimerTask>(new TimerTask(cb,Timestamp::AddCurrentTime(microsec),microsec)));
}
