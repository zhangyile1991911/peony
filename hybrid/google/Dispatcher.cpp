//
// Created by 张亦乐 on 2018/9/15.
//
#include "Dispatcher.hpp"

#include "net/TcpConnector.hpp"
#include "ProtoCallback.hpp"
#include "proto/MsgPacket.pb.h"
ProtobufDispatcher* ProtobufDispatcher::_instance = nullptr;

void ProtobufDispatcher::onProtobufMessage(
        TcpConnectorPtr conn,
        std::shared_ptr<ProtoMsg::Packet> packet
)
{
    auto iter = _callbackDict.find(packet->name());
    if(iter != _callbackDict.end())
    {
        iter->second->onMessage(conn,packet->content());
    }
    else
    {
        printf("onProtobufMessage unregister msg name = %s\n",packet->name().c_str());
    }
}

void ProtobufDispatcher::registerMessageCallback(
        std::string fullname,
        ProtoCallback* callback
)
{
    printf("register Message Callback %s\n",fullname.c_str());
    auto iter = _callbackDict.find(fullname);
    if(iter == _callbackDict.end())
    {
        _callbackDict[fullname] = callback;
    }
}