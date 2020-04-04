//
// Created by 张亦乐 on 2018/6/26.
//
#include <map>
#include <algorithm>
#include <iostream>

#include "net/Reactor.hpp"
#include "net/TcpServer.hpp"
#include "net/TcpConnectorFrom.hpp"

using namespace std;
int main()
{
    map<int,TcpConnectorPtr> ChatterDict;

    Reactor reactor;
    TcpServer server(reactor,4,"*",9876);
    server.SetInitedCallback([]{
        cout<<"broadcast服务器初始化成功"<<endl;
    });

    server.SetConnectedCallback([&ChatterDict](TcpConnectorPtr conn){
        int fd = conn->FD();
        auto iter = ChatterDict.find(fd);
        if(iter == ChatterDict.end())
        {
            ChatterDict[fd] = conn;
        }
    });

    server.SetMessageCallback([&ChatterDict](TcpConnectorPtr conn,Buffer& buff){
        string msg = buff.ReadAll();
        auto iter = ChatterDict.begin();
        for(;iter != ChatterDict.end();iter++)
        {
            //if(iter->second->FD() == conn->FD()) continue;
            iter->second->SendString(msg);
        }
    });

    server.SetClosedCallback([&ChatterDict](TcpConnectorPtr conn){
        int fd = conn->FD();
        auto iter = ChatterDict.find(fd);
        if(iter != ChatterDict.end())
        {
            ChatterDict.erase(iter);
        }
    });

    server.Start();
    reactor.Loop();

    return 0;
}