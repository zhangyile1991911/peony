//
// Created by 张亦乐 on 2018/7/6.
//

#ifndef TCPCONNECTORTO_HPP
#define TCPCONNECTORTO_HPP

#include <atomic>
#include <memory>
#include "base/Tools.hpp"
#include "TcpConnector.hpp"
//主动发起连接
class TcpConnectorTo:
        public std::enable_shared_from_this<TcpConnectorTo>,
        public TcpConnector
{
    DISALLOW_COPY_AND_ASSIGN(TcpConnectorTo)
private:
    std::atomic_bool        _connected;
    std::atomic_bool        _reachable;//非阻塞连接时候使用
    std::atomic_int         _tryCount;
private:
    static int NextWaitTime();
    void handleNetError(int);

    int  handleWriteEvent();
    int  handleReadEvent();
    void handleCloseEvent();
    void handleErrorEvent();

    void handleForceClose();
    void flushOutBuffer();
	void close();
    void rename(int fd);
    bool checkConnected();
	bool keepConnecting();
	void restart();
public:
    TcpConnectorTo(Reactor& reactor,
                   const char* name,
                   int trycount);
    ~TcpConnectorTo();
public:
    void Connect();
public:
    void ConnectEstablished()override;
    void SendString(const std::string&)override;
    void SendData(const void*,std::size_t)override;
    void ForceClose()override;
};

#endif //TCPCONNECTORTO_HPP
