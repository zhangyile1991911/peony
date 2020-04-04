//
// Created by 张亦乐 on 2018/6/22.
//

#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP

#include <functional>
#include <string>
#include <memory>

class Channel;
class Reactor;
//void(fd,ip,port)
typedef std::function<void (int,const std::string&,int) > ConnectedCallback;
class Acceptor
{
private:
    int handleReadCallback();
private:
    Reactor&                    _reactor;
    std::unique_ptr<Channel>    _channelPtr;
    ConnectedCallback           _callback;
    std::string                 _ip;
    int                         _port;
public:
    Acceptor(Reactor& reactor,
             const char* ip,
             int port,
             bool reuse_addr = false,
             bool reuse_port = false);
    ~Acceptor();

    void SetConnectedCallback(ConnectedCallback cb){_callback = std::move(cb);}
};

#endif //ACCEPTOR_HPP
