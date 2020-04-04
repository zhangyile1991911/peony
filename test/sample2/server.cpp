//
// Created by 张亦乐 on 2018/6/22.
//
#include <iostream>
#include <thread>
#include "net/Reactor.hpp"
#include "net/ReactorWorkerMgr.hpp"
#include "net/Acceptor.hpp"
#include "net/Channel.hpp"
#include "net/TcpConnectorFrom.hpp"
#include "base/Buffer.hpp"

using namespace std;
ReactorWorkerMgr* pReactorMgr;
void acceptNewClient(int fd,string ip,int port)
{
//    printf("accept new client fd = %d ip = %s port = %d\n",fd,ip.c_str(),port);
//
//    Reactor* pReactor = pReactorMgr->NextReactor();
//
//    TcpConnectorFrom* conn = new TcpConnectorFrom(pReactor,fd,ip,port);
//
//    conn->SetMessageCallback([](TcpConnectorPtr channelPtr,Buffer& buf) {
//        printf("receive data = %s\n",buf.ReadAll().c_str());
//    });
//
//    conn->SetCloseCallback([](TcpConnectorPtr pConn){
//        printf("close fd = %d\n",pConn->FD());
//    });
}

int main()
{
    Reactor reactor;

    Acceptor acceptor(reactor,"192.168.3.2",7654);

    acceptor.SetConnectedCallback(&acceptNewClient);

    ReactorWorkerMgr mgr(reactor,2);
    pReactorMgr = &mgr;

    mgr.Start();

    cout<<"start server"<<endl;
    reactor.Loop();
}