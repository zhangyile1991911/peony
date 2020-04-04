//
// Created by 张亦乐 on 2018/6/22.
//
#include <sys/socket.h>
#include <sys/errno.h>

#include <functional>

#include "plog/Log.h"
#include "base/Buffer.hpp"
#include "Channel.hpp"
#include "Reactor.hpp"
#include "TcpConnectorFrom.hpp"

using namespace std;
TcpConnectorFrom::TcpConnectorFrom(Reactor& reactor,int fd,const char* name):
        TcpConnector(reactor,fd,name),_isConnecting(false)
{
    LOGD<<"TcpConnectorFrom::TcpConnectorFrom() construct";
    _channel->SetReadCallback(bind(&TcpConnectorFrom::handleReadEvent,this));
    _channel->SetWriteCallback(bind(&TcpConnectorFrom::handleWriteEvent,this));
	_channel->SetCloseCallback(bind(&TcpConnectorFrom::handleCloseEvent,this));
}

TcpConnectorFrom::~TcpConnectorFrom()
{
    LOGD<<"TcpConnectorFrom::~TcpConnectorFrom() fd = "<<FD();
}

int TcpConnectorFrom::handleReadEvent()
{
    //从socket中读取数据
    int saveErrno = 0;
    ssize_t num = _inBuff.appendFD(FD(),saveErrno);
    if(num > 0 && _messageCallback)
    {
        _messageCallback(shared_from_this(),_inBuff);
    }
    else if(num == 0 && _closedCallback)
    {
        LOGD<<"TcpConnectorFrom::readCallback() 对端主动关闭连接";
		close();
		return FAIL;
    }
    else
    {
        LOGD<<"TcpConnectorFrom::readCallback() error!!!";
        handleNetError(saveErrno);
		return FAIL;
	}
	return OK;
}

void TcpConnectorFrom::handleNetError(int err)
{
    switch(err)
    {
        case ECONNRESET:
            //对端关闭
            _closedCallback(shared_from_this());
            break;
        case EDEADLK://EDEADLK was EAGAIN
        case EINTR://没有数据可读
            //什么都不做
            break;
        default:
            LOGD<<"handleNetError err = "<<err;
    }
}

int TcpConnectorFrom::handleWriteEvent()
{
    DisableWrite();

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
        // _closedCallback(shared_from_this());
		close();
		return FAIL;
    }
    else
    {
        //TODO 错误处理
        LOGD<<"TcpConnectorFrom::writeCallback Buffer error = "<<errno;
        handleNetError(errno);
		return FAIL;
    }
	return OK;
}

void TcpConnectorFrom::handleCloseEvent()
{
    LOGD<<"TcpConnectorFrom::handleCloseEvent()";
	close();
}

void TcpConnectorFrom::SendString(const std::string& sData)
{
    if(sData.empty())return;

	if(!_isConnecting)return;
    if(!_reactor.IsInCurrentThread())
    {//不在同一个线程,异步发送
		std::lock_guard<mutex> autoLokc(_sendMutex);
        _outBuff.Append(&sData[0],sData.size());
        _reactor.AddTask(std::bind(&TcpConnectorFrom::flushOutBuffer,shared_from_this()));
    }
    else
    {
        ssize_t num = Send(sData.c_str(), sData.size());
        unsigned long remain = sData.size() - num;
        if(remain > 0)
        {
            _outBuff.Append(&sData[num],remain);
            EnableWrite();
        }
    }
}

void TcpConnectorFrom::SendData(const void * pData,std::size_t len)
{
    if(pData == nullptr)return;

	if(!_isConnecting)return;
    if(!_reactor.IsInCurrentThread())
    {//不在同一个线程,异步发送
		std::lock_guard<mutex> autoLock(_sendMutex);
        _outBuff.Append((char*)pData,len);
        _reactor.AddTask(std::bind(&TcpConnectorFrom::flushOutBuffer,shared_from_this()));
    }
    else
    {
        std::size_t num = Send(pData, len);
        std::size_t remain = len - num;
        if(remain > 0)
        {
            _outBuff.Append(((char*)pData+num),remain);
            EnableWrite();
        }
    }
}

void TcpConnectorFrom::flushOutBuffer()
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
        }
    }
    else if(num == 0)
    {
        //对端关闭
        LOGD<<"TcpConnectorFrom::Send() close socket";
		close();
    }
    else
    {
        //TODO 错误处理
        LOGD<<"TcpConnectorFrom::sendBuffer error = "<<errno;
    }
}

void TcpConnectorFrom::ConnectEstablished()
{
    _channel->EnableRead();
    _isConnecting = true;
    if(_establishedCallback) _establishedCallback(shared_from_this());
}

void TcpConnectorFrom::ForceClose()
{
    if(_reactor.IsInCurrentThread())
    {
		close();
    }
    else
    {
        _reactor.AddTask(std::bind(&TcpConnectorFrom::handleForceClose,this));
    }
}

void TcpConnectorFrom::handleForceClose()
{
	close();
}
void TcpConnectorFrom::close()
{
	_isConnecting = false;
	if(_closedCallback)
	    _closedCallback(shared_from_this());
}
