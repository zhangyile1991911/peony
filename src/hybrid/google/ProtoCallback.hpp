//
// Created by 张亦乐 on 2018/9/15.
//

#ifndef PEONY_PROTOCALLBACK_HPP
#define PEONY_PROTOCALLBACK_HPP

#include <functional>
#include <string>
#include <sys/time.h>
#include "net/TcpConnector.hpp"
#include "Dispatcher.hpp"

#include "proto/MsgPacket.pb.h"
//所有消息处理回调接口
class ProtoCallback
{
public:
    virtual ~ProtoCallback(){}
    virtual void onMessage(TcpConnectorPtr,const std::string&) = 0;
};

template<typename Object>
using ConvertTie = std::function<Object(TcpConnectorPtr)>;

template<typename T,typename Object = TcpConnectorPtr>
class ProtoCallbackT : public ProtoCallback
{
public:
    using TemplateCallback = std::function<void(
            Object obj,
            const T&
            )>;

    explicit ProtoCallbackT(
            TemplateCallback cb,
            ConvertTie<Object> convert
                    ):_callback(cb),_convert(convert)
    {
        ProtobufDispatcher::GetInstance()->registerMessageCallback
        (
                T::descriptor()->full_name(),
                this
        );
    }
    ~ProtoCallbackT(){}

    void onMessage(TcpConnectorPtr conn,const std::string& content)override
    {
        T request;
        request.ParseFromString(content);
        //todo 反序列化失败处理
        //printf("----------\n%s\n%s++++++++++\n",T::descriptor()->full_name().c_str(),request.DebugString().c_str());

        if(_convert != nullptr)
        {//将TcpConnector上的挂件Tie转换成具体用户对象
            _callback(_convert(conn),request);
        }
        else
        {
            printf("error ProtoCallbackT::onMessage %s no convert function",T::descriptor()->full_name().c_str());
        }
    }
protected:
	static	ProtoCallbackT<T,Object>	Instance;
private:
    TemplateCallback			_callback;
    ConvertTie<Object>          _convert;
};

template<typename IN,typename OUT,typename Object = TcpConnectorPtr>
class ProtoCallbackReply : public ProtoCallback
{
public:
    using TemplateCallbackReply = std::function<int32_t(
            Object,
            const IN&,
            OUT&
            )>;

    explicit ProtoCallbackReply(
            TemplateCallbackReply cb,
            ConvertTie<Object> convert
                    ):_callback(cb),_convert(convert)
    {
        ProtobufDispatcher::GetInstance()->registerMessageCallback
        (
                IN::descriptor()->full_name(),
                this
        );
    }

    void onMessage(TcpConnectorPtr conn,const std::string& content)
    {
		struct timeval tv;
		gettimeofday(&tv, nullptr);
		uint64_t startms  = tv.tv_sec*1000 + tv.tv_usec/1000;
        const std::string& requestName = IN::descriptor()->full_name().c_str();
		IN request;
        request.ParseFromString(content);
        //todo 反序列化失败处理
        //printf("----------\n%s\n%s++++++++++\n",IN::descriptor()->full_name().c_str(),request.DebugString().c_str());

        OUT response;
        int32_t code = 0;
        if(_convert != nullptr)
        {
            code = _callback(_convert(conn),request,response);
        }
        else
        {
            printf("error ProtoCallbackReply::onMessage %s no convert function",requestName.c_str());
        }

        //printf("----------\n%s\ncode=%d\n%s++++++++++\n",OUT::descriptor()->full_name().c_str(),code,response.DebugString().c_str());

        ProtoMsg::Packet packet;
        packet.set_name(OUT::descriptor()->full_name());
        packet.set_content(response.SerializeAsString());
        packet.set_seq(0);
		packet.set_timestamp(startms);
        packet.set_code(code);

        PackMsgToNetData(conn,packet);
		bzero(&tv,sizeof(struct timeval));
		gettimeofday(&tv,nullptr);
		uint64_t endms = tv.tv_sec*1000 + tv.tv_usec/1000;
		printf("ProtoCallbackReplay %s time spent %ld ms\n",requestName.c_str(),(endms - startms));
    }

    void PackMsgToNetData(TcpConnectorPtr conn,ProtoMsg::Packet& packet)
    {
        //|payload(4)|protobuf(x)   |
        //|后面协议长度|具体协议数据     |
        int32_t payloadSize = packet.ByteSize();
        int32_t totalSize = payloadSize + 4;
        std::unique_ptr<char> netDataPtr(new char[totalSize]);
        memcpy(&netDataPtr.get()[0],&payloadSize,4);
        //todo 序列化失败处理
        packet.SerializeToArray(&netDataPtr.get()[4],payloadSize);
        if(conn)
        {
            conn->SendData(netDataPtr.get(),totalSize);
        }
    }
protected:
	static ProtoCallbackReply<IN,OUT,Object>	Instance;
private:
    TemplateCallbackReply				_callback;
    ConvertTie<Object>                  _convert;
};
#endif //PEONY_PROTOCALLBACK_HPP
