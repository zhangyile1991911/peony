//
// Created by 张亦乐 on 2018/7/6.
//

#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include "plog/Log.h"
#include "TcpConnectorTo.hpp"
#include "NetTools.hpp"
using namespace std;
TcpConnectorTo::TcpConnectorTo(Reactor &reactor,
                               const char *name,
                               int trycount):
        TcpConnector(reactor,name),
        _connected(false),
        _reachable(true),
        _tryCount(trycount)
{
}

TcpConnectorTo::~TcpConnectorTo()
{
    LOGD<<"~TcpConnectorTo "<<Name();
}

void TcpConnectorTo::Connect()
{
    int fd = ::socket(AF_INET,SOCK_STREAM,0);
    set_noblock_fd(fd);
    rename(fd);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(_remoteIP.c_str());
    addr.sin_port = htons(_remotePort);

    _channel.reset(new Channel(_reactor,fd));
    _channel->SetReadCallback(std::bind(&TcpConnectorTo::handleReadEvent,this));
    _channel->SetWriteCallback(std::bind(&TcpConnectorTo::handleWriteEvent,this));
#ifdef __APPLE__
    _channel->SetCloseCallback(std::bind(&TcpConnectorTo::handleCloseEvent,this));
    _channel->SetErrorCallback(std::bind(&TcpConnectorTo::handleErrorEvent,this));
#elif __linux__
    _channel->SetErrorCallback(std::bind(&TcpConnectorTo::handleErrorEvent,this));
    _channel->SetCloseCallback(std::bind(&TcpConnectorTo::handleCloseEvent,this));
#endif
    _connected = false;
    _reachable = true;
    LOGD<<"now "<<Timestamp::Now().Epoch()/PerMiscroSeconds;
    int result = ::connect(FD(),(sockaddr*)&addr, sizeof(struct sockaddr_in));
    if(result == 0 && _establishedCallback)
    {//同一台机器,连接瞬间建立
        checkConnected();
        _establishedCallback(shared_from_this());
    }
    else
    {//非阻塞,等待套接字可写,连接建立成功
        LOGD<<"TcpConnectorTo error = "<<strerror(errno);
        _channel->EnableWrite();
    }
}

int TcpConnectorTo::handleWriteEvent()
{
    if(!_connected && !checkConnected())
    {
        return OK;
    }

    if(_outBuff.Empty())
    {
        return OK;
    }

    ssize_t num = Send(_outBuff.ReadPos(), _outBuff.ReadableBytes());
    if(num > 0)
    {
        if(_outBuff.ReadableBytes() == 0)
        {
            DisableWrite();
        }
        _outBuff.Read(num);
    }
    else if(num == 0)
    {
		close();
		return FAIL;
    }
    else
    {
        //TODO 错误处理
        LOGD<<"TcpConnectorTo::writeCallback Buffer error = "<<errno;
        handleNetError(errno);
		return FAIL;
    }
	return OK;
}

int TcpConnectorTo::handleReadEvent()
{
    if(!_connected)return OK;
    //从socket中读取数据
    int saveErrno = 0;
    ssize_t num = _inBuff.appendFD(FD(),saveErrno);
    if(num > 0 && _messageCallback)
    {
        _messageCallback(shared_from_this(),_inBuff);
    }
    else if(num == 0 && _closedCallback)
    {
        LOGD<<"TcpConnectorTo::readCallback() 对端主动关闭连接,尝试重新连接";
		close();
        restart();
    }
    else
    {
        LOGD<<"TcpConnectorTo::readCallback() error!!!";
        handleNetError(saveErrno);
		return FAIL;
    }
	return OK;
}

void TcpConnectorTo::handleCloseEvent()
{
    if(!keepConnecting())
    {//尝试重连次数用完
        if(_closedCallback)
        {
			close();
        }
    }

}

void TcpConnectorTo::handleErrorEvent()
{
    _reachable = false;
    LOGD<<"TcpConnectorTo::handleCloseEvent "<<Name()<<" _connected "<<_connected.load();
    if(!checkConnected())
    {
        if(!keepConnecting())
        {
			close();
        }
    }
}

void TcpConnectorTo::handleNetError(int err)
{
    LOGD<<"TcpConnectoTo::handleNetError() "<<err;
}

void TcpConnectorTo::SendString(const std::string& strData)
{
    if(strData.empty())return;
	if(!_connected.load())return;
    std::lock_guard<mutex> autoLock(_sendMutex);
    if(!_reactor.IsInCurrentThread())
    {//不在同一个线程,异步发送
        _outBuff.Append(&strData[0],strData.size());
        _reactor.AddTask(std::bind(&TcpConnectorTo::flushOutBuffer,shared_from_this()));
    }
    else
    {
        ssize_t num = Send(strData.c_str(), strData.size());
        unsigned long remain = strData.size() - num;
        if(remain > 0)
        {
            _outBuff.Append(&strData[num],remain);
            EnableWrite();
            //_channel->EnableWrite();
        }
    }
}

void TcpConnectorTo::SendData(const void * pData,std::size_t len)
{
    if(pData == nullptr)return;
	if(!_connected.load())return;
    std::lock_guard<mutex> autoLock(_sendMutex);
    if(!_reactor.IsInCurrentThread())
    {//不在同一个线程,异步发送
        _outBuff.Append((char*)pData,len);
        _reactor.AddTask(std::bind(&TcpConnectorTo::flushOutBuffer,shared_from_this()));
    }
    else
    {
        std::size_t num = Send(pData, len);
        std::size_t remain = len - num;
        if(remain > 0)
        {
            _outBuff.Append(((char*)pData+num),remain);
            EnableWrite();
            //_channel->EnableWrite();
        }
    }
}

void TcpConnectorTo::flushOutBuffer()
{
    //将outBuffer内容发送
    if(_outBuff.Empty())
    {
        return;
    }

    ssize_t num = Send(_outBuff.ReadPos(), _outBuff.ReadableBytes());
    if(num > 0)
    {
        _outBuff.HasRead(num);
        if(_outBuff.ReadableBytes() > 0)
        {
            EnableWrite();
            //_channel->EnableWrite();
        }
    }
    else if(num == 0)
    {
        //对端关闭
        LOGD<<"TcpConnectorTo::Send() close socket";
        _closedCallback(shared_from_this());
    }
    else
    {
        //TODO 错误处理
        LOGD<<"TcpConnectorTo::sendBuffer error = "<<errno;
    }
}

void TcpConnectorTo::ConnectEstablished()
{

}

void TcpConnectorTo::ForceClose()
{
    if(_reactor.IsInCurrentThread())
    {
		close();
    }
    else
    {
        _reactor.AddTask(std::bind(&TcpConnectorTo::handleForceClose,this));
    }
}

void TcpConnectorTo::handleForceClose()
{
	close();
}

void TcpConnectorTo::rename(int fd)
{
    char buff[50];
    snprintf(buff,50,"%s:%d-%d",_remoteIP.c_str(),_remotePort,fd);
    std::string newName(buff);
    _name.swap(newName);
}

bool TcpConnectorTo::checkConnected()
{
    LOGD<<"reachable "<<_reachable.load();
    if(_reachable)
    {
        LOGD<<"TcpConnectorTo::checkConnected successful";
        _connected = true;
        _channel->EnableRead();
        _channel->DisableWrite();
        _establishedCallback(shared_from_this());
        return true;
    }
	return false;
}

bool TcpConnectorTo::keepConnecting()
{
    //失败尝试重连
    if(_tryCount == -1)
    {//无限尝试重连
        LOGD<<Name()<<"无限尝试重连";
        restart();
        return true;
    }
    else if(_tryCount == 0)
    {//重连次数用完,结束
        LOGD<<Name()<<"尝试重连,次数用完";

        handleForceClose();
        return false;
    }
    else
    {
        _tryCount--;
        restart();
        LOGD<<Name()<<" 尝试重连,剩余次数 "<<_tryCount.load();
        return true;
    }
}

void TcpConnectorTo::restart()
{
    _reactor.AddTask([this](){
        _channel.reset(nullptr);
    });

    _reactor.RunAfter([this](){
        Connect();
    },NextWaitTime()*PerMiscroSeconds);
}

int TcpConnectorTo::NextWaitTime()
{
    static int count = 1;
    int nextTime = count*5;
    LOGD<<"next wait time "<<nextTime;
    return nextTime;
}

void TcpConnectorTo::close()
{
	_connected = false;
	if(_closedCallback)_closedCallback(shared_from_this());
}
