//
// Created by zhangyile on 18-6-23
//
#include <csignal>

#include "plog/Log.h"

#include "net/Channel.hpp"
#include "net/Reactor.hpp"
#include "net/Acceptor.hpp"
#include "net/ReactorWorkerMgr.hpp"
#include "net/TcpConnectorFrom.hpp"
#include "net/TcpServer.hpp"

using namespace std;
using namespace std::placeholders;
TcpServer::TcpServer(Reactor& reactor,
                     int workerNum,
                     const char* ip,
                     int port,
                     bool reuseAddr,
                     bool reusePort):
        _ip(ip),
        _port(port),
        _createCallback(nullptr),
        _acceptorPtr(new Acceptor(reactor,ip,port,reuseAddr,reusePort)),
        _managerPtr(new ReactorWorkerMgr(reactor,workerNum)),
        _baseReactor(reactor)
{
    ::signal(SIGPIPE,SIG_IGN);
}

TcpServer::~TcpServer()
{

}

void TcpServer::Start()
{
    _acceptorPtr->SetConnectedCallback(std::bind(&TcpServer::handleAcceptRead,this,_1,_2,_3));

    _managerPtr->Start();

    _started = true;

    if(_initedCallback != nullptr) _initedCallback();
}

void TcpServer::handleAcceptRead(int fd,const string& ip, int port)
{
    _baseReactor.AssertCurrentThread();

    char buff[50];
    snprintf(buff,50,"%s:%d-%d",ip.c_str(),port,fd);

    Reactor* pReactor = _managerPtr->NextReactor();

    TcpConnectorPtr connPtr;
    if(_createCallback)
    {
        connPtr = _createCallback(*pReactor,fd,buff);
    }
    else
    {
        connPtr = TcpConnectorPtr(new TcpConnectorFrom(*pReactor,fd,buff));
    }

    if(connPtr == nullptr)
    {
        ::close(fd);
        return;
    }

    connPtr->SetMessageCallback(std::bind(&TcpServer::handleMessageConn, this, _1, _2));
    connPtr->SetCloseCallback(std::bind(&TcpServer::handleCloseConn, this, _1));
    connPtr->SetEstablishCallback(std::bind(&TcpServer::handleConnectedConn, this, _1));

    {//保证map线程安全
        lock_guard<mutex> autoLock(_connMutex);
        _connDict[connPtr->Name()] = connPtr;
    }

    pReactor->AddTask(std::bind(&TcpConnector::ConnectEstablished,connPtr));
}

//多线程重入函数
void  TcpServer::handleConnectedConn(TcpConnectorPtr tcpConn)
{
    //这里用锁,外层操作都是单线程
    lock_guard<mutex> autoLock(_connMutex);
    LOGD<<"TcpServer::handleConnectedConn tcpConn "<<tcpConn.use_count();
    if(_connectedCallback) _connectedCallback(tcpConn);
    LOGD<<"TcpServer::handleConnectedConn _connectedCallback "<<tcpConn.use_count();
}

//多线程重入函数
void TcpServer::handleCloseConn(TcpConnectorPtr tcpConn)
{
    //这里用锁,外层操作都是单线程
    lock_guard<mutex> autoLock(_connMutex);
    auto iter = _connDict.find(tcpConn->Name());
    if(iter != _connDict.end())
    {
        LOGD<<"_connDict erase  = "<<tcpConn->Name();
        _connDict.erase(iter);
    }

    if(_closedCallback) _closedCallback(tcpConn);
    // printf("TcpServer::handleCloseConn tcpConn.use_count = %ld\n",tcpConn.use_count());
}

void TcpServer::handleMessageConn(TcpConnectorPtr tcpConn,Buffer& buffer)
{
    if(_messageCallback) _messageCallback(tcpConn,buffer);
}
