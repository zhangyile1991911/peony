//
// Created by 张亦乐 on 2018/9/15.
//

#ifndef PEONY_DISPATCHER_HPP
#define PEONY_DISPATCHER_HPP

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace ProtoMsg
{
    class Packet;
}
class ProtoCallback;
class TcpConnector;
using TcpConnectorPtr = std::shared_ptr<TcpConnector>;

class ProtobufDispatcher
{
public:
    void onProtobufMessage(TcpConnectorPtr conn,std::shared_ptr<ProtoMsg::Packet>);
    void registerMessageCallback(std::string fullname,ProtoCallback* callback);

public:
    static ProtobufDispatcher* GetInstance()
    {
        if(_instance == nullptr)
        {
            _instance = new ProtobufDispatcher();
        }
        return _instance;
    }

protected:
    ProtobufDispatcher() = default;
    ~ProtobufDispatcher() = default;

private:
    using CallbackDict = std::unordered_map<std::string,ProtoCallback*>;
    CallbackDict                _callbackDict;
    static ProtobufDispatcher*  _instance;

};

#endif //PEONY_DISPATCHER_HPP
