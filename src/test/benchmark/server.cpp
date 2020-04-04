#include<unistd.h>
#include<sys/time.h>
#include<cstdio>
#include<iostream>
#include<functional>
#include<atomic>
#include<thread>

#include "net/Reactor.hpp"
#include "net/TcpServer.hpp"
#include "net/TcpConnectorFrom.hpp"
#include "lotus/ConcurrentQueue.hpp"
using namespace std;
void Network_Traffic_Statistics();
void ConcurrentQueue_Perform();
int main()
{
    //Network_Traffic_Statistics();
    ConcurrentQueue_Perform();
	return 0;	
}

void Network_Traffic_Statistics()
{
    atomic_int byteCount(0);
    Reactor reactor;
    TcpServer server(reactor,4,"172.26.192.170",9999);
    server.SetInitedCallback([]{
        cout<<"服务器初始化成功"<<endl;
    });
    server.SetConnectedCallback([](TcpConnectorPtr conn){
    });
    server.SetMessageCallback(
            [&byteCount](TcpConnectorPtr conn,Buffer& buff){
                auto data = buff.ReadAll();
                byteCount.fetch_add(data.size());
                conn->SendString(std::move(data));
            });
    server.SetClosedCallback(
            [](TcpConnectorPtr conn){
                //              printf("TcpConnectorFrom use_count = %ld fd = %d will be closed\n",
                //                     conn.use_count(),conn->FD());
            });
    reactor.RunRepeated(
            [&byteCount](){
                struct timeval tv;
                gettimeofday(&tv,nullptr);
                printf("now tv_sec %ld receive byte count %d mb\n",tv.tv_sec,byteCount.load()/1024/1024);
            },5*PerMiscroSeconds);
    server.Start();
    reactor.Loop();
}

ConcurrentQueue<int> cQueue;
void prudctor()
{
    while(true)
    {
        cQueue.push(10);
    }
}
void ConcurrentQueue_Perform()
{
    thread productor[4]{
        thread(&prudctor),
        thread(&prudctor),
        thread(&prudctor),
        thread(&prudctor)
    };
    Reactor reactor;
    reactor.RunRepeated(
            [](){
                struct timeval tv;
                gettimeofday(&tv,nullptr);
                printf("now tv_sec %ld push count %lu\n",tv.tv_sec,cQueue.PushNum.load());
            },5*PerMiscroSeconds);
    reactor.Loop();
}

