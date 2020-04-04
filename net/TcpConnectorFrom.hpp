//
// Created by 张亦乐 on 2018/6/22.
//

#ifndef TCPCONNECTORFROM_HPP
#define TCPCONNECTORFROM_HPP

#include <functional>
#include <memory>
#include <string>
#include <mutex>
#include <atomic>
#include <cstddef>

#include "base/Tools.hpp"
#include "base/Buffer.hpp"
#include "base/Callback.hpp"
#include "net/Channel.hpp"
#include "net/TcpConnector.hpp"
class Reactor;
//TcpConnector用于被动接收连接
class TcpConnectorFrom:
        public std::enable_shared_from_this<TcpConnectorFrom>,
        public TcpConnector
{
DISALLOW_COPY_AND_ASSIGN(TcpConnectorFrom)
private:
	std::atomic_bool _isConnecting;
private:
    void handleNetError(int);
    int  handleReadEvent();
    int  handleWriteEvent();
	void handleCloseEvent();
    void handleForceClose();
    void flushOutBuffer();
	void close();
public:
    TcpConnectorFrom(Reactor& reactor,int fd,const char* name);
    ~TcpConnectorFrom();
public:
    //只有在将channel注册到epoll/kqueue中后才算连接建立完成
    void ConnectEstablished()override;
    void SendString(const std::string&)override;
    void SendData(const void*,std::size_t)override;
    void ForceClose()override;
};
#endif //TCPCONNECTORFROM_HPP
