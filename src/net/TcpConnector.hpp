//
// Created by 张亦乐 on 2018/7/6.
//

#ifndef TCPCONNECTOR_HPP
#define TCPCONNECTOR_HPP

#include <string>
#include <memory>
#include "base/Buffer.hpp"
#include "base/Callback.hpp"
#include "Reactor.hpp"
#include "Channel.hpp"
class TcpConnector
{
protected:
    std::unique_ptr<Channel>	_channel;
    Reactor&					_reactor;

    std::string					_name;//127.0.0.1:9876-10
    std::string					_remoteIP;
    int							_remotePort;

    Buffer						_outBuff;
    Buffer						_inBuff;

    void*		                _tie;//挂件

    ConnectorCallback			_establishedCallback;
    ConnectorCallback			_closedCallback;
    MessageCallback				_messageCallback;

    mutable std::mutex			_sendMutex;
    std::atomic_bool			_listenWrite;

protected:
    void ParseIpPort()
    {
        //172.26.192.170:98761-65535
        std::string::size_type semicolon = _name.find(":");
        _remoteIP = _name.substr(0,semicolon);

        std::string::size_type  dash = _name.find("-");
        _remotePort = atoi(_name.substr(semicolon+1,dash- 1 - semicolon).c_str());

        //printf("TcpConnector = %s %d \n",_remoteIP.c_str(),_remotePort);
    }
    void EnableWrite()
    {
        if(!_listenWrite)
        {
            _channel->EnableWrite();
            _listenWrite = true;
        }
    }
    void DisableWrite()
    {
        if(_listenWrite)
        {
            _channel->DisableWrite();
            _listenWrite = false;
        }
    }
    ssize_t Send(const void *pData, std::size_t len)
    {
        ssize_t result = ::write(_channel->GetFd(),pData,len);
        // printf("TcpConnector::Send len = %ld result = %ld\n",len,result);
        return result;
    }
public:
    TcpConnector(Reactor& reactor,int fd,const char* name):
    _channel(new Channel(reactor,fd)),
    _reactor(reactor),
    _name(name),
    _listenWrite(false)
    {
        ParseIpPort();
    }
    TcpConnector(Reactor& reactor,const char* name):
            _reactor(reactor),
            _name(name),
            _listenWrite(false)
    {
        ParseIpPort();
    }
    virtual ~TcpConnector()
    {
        _tie = nullptr;
    }
public:
    void SetEstablishCallback(ConnectorCallback cb){_establishedCallback = std::move(cb);}
    void SetMessageCallback(MessageCallback cb){_messageCallback = std::move(cb);}
    void SetCloseCallback(ConnectorCallback cb){_closedCallback = std::move(cb);}
public:
    virtual void SendString(const std::string&) = 0;
    virtual void SendData(const void*,std::size_t) = 0;
    virtual void ForceClose() = 0;
    virtual void ConnectEstablished() = 0;
public:
    void SetTie(void* tie){_tie = tie;}
    void* GetTie(){return _tie;}
    int FD()const{return _channel->GetFd();}
    int Port()const{return _remotePort;}
    const std::string& RemoteIP(){return _remoteIP;}
    const std::string& Name(){return _name;}
};
typedef std::shared_ptr<TcpConnector>  TcpConnectorPtr;
#endif //TCPCONNECTOR_HPP
