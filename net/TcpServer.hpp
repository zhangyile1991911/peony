//
// Created by 张亦乐 on 2018/6/22.
//

#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <functional>
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "base/Tools.hpp"
#include "net/TcpConnectorFrom.hpp"
#include "base/Callback.hpp"

//向前声明
class Buffer;
class Reactor;
class Acceptor;
class ReactorWorkerMgr;

typedef std::unordered_map<std::string,TcpConnectorPtr> ConnectionDict;
//为了在构造shared_ptr<TcpConnector>传入不同的Deleter
using CreateCallback = std::function<TcpConnectorPtr(Reactor&,int,char*)>;
class TcpServer
{
    DISALLOW_COPY_AND_ASSIGN(TcpServer)
private:
    const std::string       _ip;
    const int               _port;
    bool                    _started;
private:
    std::mutex                          _connMutex;
    ConnectionDict                      _connDict;//会被多个线程同时调用
private:
    std::unique_ptr<Acceptor>           _acceptorPtr;
    std::unique_ptr<ReactorWorkerMgr>   _managerPtr;//TODO 改个更好的名字?
    Reactor&                            _baseReactor;
private:
    ConnectorCallback                   _connectedCallback;
    ConnectorCallback                   _closedCallback;
    Callback                            _initedCallback;
    MessageCallback                     _messageCallback;
    CreateCallback                      _createCallback;
private:
    void  handleAcceptRead(int,const std::string&,int);
    void  handleConnectedConn(TcpConnectorPtr);
    void  handleMessageConn(TcpConnectorPtr,Buffer&);
    void  handleCloseConn(TcpConnectorPtr);
public:
    TcpServer(Reactor& reactor,int workerNum,
              const char* ip,int port,bool reuseAddr = false,bool reusePort = false);
    ~TcpServer();

    void SetInitedCallback(Callback cb){_initedCallback = std::move(cb);}
    void SetConnectedCallback(ConnectorCallback cb){_connectedCallback = std::move(cb);}
    void SetClosedCallback(ConnectorCallback cb){_closedCallback = std::move(cb);}
    void SetMessageCallback(MessageCallback cb){_messageCallback = std::move(cb);}
    void SetCreateCallback(CreateCallback cb){_createCallback = std::move(cb);}

    void Start();
    bool isStart(){return _started;}
};
#endif //TCPSERVER_HPP
