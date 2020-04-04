//
// Created by 张亦乐 on 2018/6/21.
//
#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <sys/epoll.h>
static const int READ_EVENT = EPOLLIN;
static const int WRITE_EVENT = EPOLLOUT;
static const int NONE_EVENT = 0;


#include <atomic>
#include <functional>
#include <queue>
#include "base/Callback.hpp"

//向前声明
class Reactor;

//Channel   生命周期由包含他的类决定
//Channel   只是简单时间回调分发类,不要继承,应该采用has-a
class Channel
{
private:
    Reactor&    _reactor;
    int         _fd;
    int         _event;//用户自定义监听事件
    int         _revent;//实际系统发生的事件
    bool        _join;//是否加入kqueue/epoll
    //MACOS 使用字段
    uint16_t    _flag;//KQUEUE中使用来判断socket出错事件
    uint16_t    _flags;//KQUEUE中使用,EV_ADD,EV_DELETE,EV_ENABLE,EV_DISABLE
    int         _custom_event;//KQUEUE中使用
    long        _data;
    uint32_t    _fflags;

	static std::atomic_int  ChannelNum;
private:
    ICallback       _readCallback;
    ICallback       _writeCallback;
    Callback       _closeCallback;
    Callback       _errorCallback;
private:
    void        update();
    void        handleEpoll();
public:
    Channel(Reactor& reactor,int fd);
    ~Channel();

    void SetReadCallback(ICallback cb){_readCallback = std::move(cb);}
    void SetWriteCallback(ICallback cb){_writeCallback = std::move(cb);}
    void SetCloseCallback(Callback cb){_closeCallback = std::move(cb);}
    void SetErrorCallback(Callback cb){_errorCallback = std::move(cb);}

    int        GetFd()const {return _fd;}
    int        GetEvent()const{return _event;}


    void        HandleEvent();
    //往Reactor中注册事件
    void        EnableRead(int64_t microsecond = 0);//只有当Channel是Timer时,微妙
    void        EnableWrite();

    void        DisableRead();
    void        DisableWrite();;
    void        DisableAll();

    void        SetRevent(int revent,uint16_t flag);
    
    bool        CanWrite();
};

#endif //CHANNEL_HPP
