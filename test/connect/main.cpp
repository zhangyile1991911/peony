//
// Created by 张亦乐 on 2018/7/6.
//
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include "base/Buffer.hpp"
#include "net/TcpConnectorTo.hpp"
#include "net/Reactor.hpp"
#include "net/NetTools.hpp"
const char* ip = "172.26.192.170";
using namespace std;
int main()
{

    Reactor reactor;
    char buff[50];
    snprintf(buff,50,"%s:%d-",ip,9876);
    shared_ptr<TcpConnectorTo> tcpConnectorTo(new TcpConnectorTo(reactor,buff,10));
    tcpConnectorTo->Connect();
    tcpConnectorTo->SetEstablishCallback([](TcpConnectorPtr connPtr){
        printf("connected %s success!!!\n",ip);
        connPtr->SendData("hello",5);
    });
    tcpConnectorTo->SetMessageCallback([](shared_ptr<TcpConnector> connPtr,Buffer& buffer) {
        printf("name: %s receive %s\n",connPtr->Name().c_str(),buffer.ReadAll().c_str());
    });
    tcpConnectorTo->SetCloseCallback([](shared_ptr<TcpConnector> connPtr){
        printf("name: %s disconnect\n",connPtr->Name().c_str());
    });
    reactor.Loop();
	return 0;
}
