//
// Created by 张亦乐 on 2018/6/22.
//
#include <functional>

#include "net/Reactor.hpp"
#include "net/Acceptor.hpp"
#include "net/Channel.hpp"
using namespace std;
Reactor gReactor;
int acceptNewClient(int fd,string ip,int port)
{
    printf("accept new client fd = %d ip = %s port = %d\n",fd,ip.c_str(),port);
    Channel* channel = new Channel(gReactor,fd);
    channel->EnableRead();

    channel->SetReadCallback([&fd]()->int{
        printf("fd = %d has read event\n",fd);
		return OK;
    });
    channel->SetCloseCallback([&fd](){
        printf("fd = %d has close event\n",fd);
    });
	return 0;
}
void TestReactor()
{
    Acceptor acceptor(gReactor,"127.0.0.1",7654,true,true);

    acceptor.SetConnectedCallback(&acceptNewClient);

    gReactor.Loop();
}

int main()
{
    TestReactor();
}

