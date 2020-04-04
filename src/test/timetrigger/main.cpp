//
// Created by 张亦乐 on 2018/6/28.
//

#include <unistd.h>
#include <sys/time.h>

#include <iostream>
#include <functional>
#include <vector>
#include <atomic>
#include <map>

#include "base/Callback.hpp"
#include "net/Reactor.hpp"
#include "net/Channel.hpp"
#include "net/Timer.hpp"
using namespace std;

int main()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    cout<<"tv_sec "<<tv.tv_sec<<" tv_usec "<<tv.tv_usec<<endl;
    Reactor reactor;
    Timer   timer(reactor);
    int     count = 5;
    timer.ResetOnce(5*1000*1000);
    timer.SetTimerCallback([&count](Timer& rt){
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        cout<<"tv_sec "<<tv.tv_sec<<" tv_usec "<<tv.tv_usec<<endl;
        count--;
        switch (count)
        {
            case 1:
                rt.ResetOnce(1*1000*1000);
                break;
            case 2:
                rt.ResetOnce(2*1000*1000);
                break;
            case 3:
                rt.ResetOnce(3*1000*1000);
                break;
            case 4:
                rt.ResetOnce(4*1000*1000);
                break;
        }
    });
    reactor.Loop();
}