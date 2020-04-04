//
// Created by zhangyile on 18-7-13.
//
#include <unistd.h>
#include <sys/time.h>

#include <iostream>
#include <functional>

#include "net/Reactor.hpp"
#include "net/TimerQueue.hpp"

using namespace std;

void PrintOnce()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    cout<<"PrintOnce tv_sec "<<tv.tv_sec<<endl;
}
void PrintFiveSecond()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    cout<<"PrintFiveSecond tv_sec "<<tv.tv_sec<<endl;
}
void PrintTenSecond()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    cout<<"PrintTenSecond tv_sec "<<tv.tv_sec<<endl;
}

int main()
{
    Reactor reactor;
    //TimerQueue tq(reactor);
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    cout<<"now tv_sec "<<tv.tv_sec<<" tv_usec "<<tv.tv_usec<<" now_ms "<<tv.tv_sec*1000*1000+tv.tv_usec<<endl;
    cout<<"----------------------------------------"<<endl;
//    tq.AddTimerTask(PrintPerFiveSecond,Timestamp::AddCurrentTime(5*1000*1000),5*1000*1000);//每隔5秒执行一次
//    tq.AddTimerTask(PrintOnce,Timestamp::AddCurrentTime(2*1000*1000),0);//等2秒后执行一次
//    tq.AddTimerTask(PrintTenSecond,Timestamp::AddCurrentTime(10*1000*1000),5*1000*1000);//等10秒后执行一次,之后每5秒执行一次

    reactor.RunAfter(PrintOnce,2*PerMiscroSeconds);
    reactor.RunAt(PrintFiveSecond,Timestamp::AddCurrentTime(5*PerMiscroSeconds));
    reactor.RunRepeated(PrintTenSecond,10*PerMiscroSeconds);
    reactor.Loop();
    return 0;
}