//
// Created by 张亦乐 on 2018/6/21.
//

#ifndef REACTORWORKER_HPP
#define REACTORWORKER_HPP

#include <functional>
#include <string>
class Reactor;
class Worker;
class WaitGroup;

typedef std::function<void(const std::string&)>    InitCallback;
class ReactorWorker
{
private:
    std::string     _name;
    Worker*         _worker;
    Reactor*        _reactor;
    InitCallback    _initCallback;
private:
    void            doWork();
public:
    ReactorWorker(char*);
    ~ReactorWorker();

    void SetInitCallback(InitCallback cb){_initCallback = cb;}
    void StartWork();

    Reactor* GetReactor()const{return _reactor;}
};

#endif //REACTORWORKER_HPP
