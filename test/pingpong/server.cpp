//
// Created by 张亦乐 on 2018/6/21.
//
#include <functional>
#include <string>
#include <cstdio>
#include <memory>
#include "base/Buffer.hpp"
#include "net/Reactor.hpp"
#include "net/TcpServer.hpp"
#include "net/TcpConnectorFrom.hpp"

using namespace std;

int main()
{
    Reactor reactor;
    TcpServer server(reactor,2,"172.26.192.175",9876);
    server.SetInitedCallback([]{
        printf("服务器初始化成功\n");
    });
    server.SetConnectedCallback([](TcpConnectorPtr conn){
        printf("new TcpConnectorFrom use_count = %ld fd = %d ip = %s port = %d\n",
        conn.use_count(),conn->FD(),conn->RemoteIP().c_str(),conn->Port());
    });
    server.SetMessageCallback(
            [](TcpConnectorPtr conn,Buffer& buff){
                auto data = buff.ReadAll();
                printf("TcpConnectorFrom use_count = %ld fd = %d receive data = %s\n",
                conn.use_count(),conn->FD(),data.c_str());
                conn->SendString(std::move(data));
            });
    server.SetClosedCallback(
            [](TcpConnectorPtr conn){
                printf("TcpConnectorFrom use_count = %ld fd = %d will be closed\n",
                       conn.use_count(),conn->FD());
            });
    server.Start();
    reactor.Loop();
	return 0;
}
