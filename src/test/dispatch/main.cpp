//
// Created by 张亦乐 on 2018/9/15.
//
#include <string>
#include <memory>

#include "hybrid/google/Dispatcher.hpp"
#include "hybrid/google/ProtoCallback.hpp"

#include "proto/MsgGreet.pb.h"
#include "proto/MsgPacket.pb.h"
using namespace std;
using namespace google;
using namespace google::protobuf;

using namespace ProtoMsg;

TcpConnectorPtr NoConvert(TcpConnectorPtr conn)
{
    return conn;
}
#define HANDLE_PROTO_MSG(REQUEST)\
void Handle##REQUEST(TcpConnectorPtr conn,const REQUEST& request);\
template<>\
ProtoCallbackT<REQUEST> ProtoCallbackT<REQUEST>::Instance(Handle##REQUEST,NoConvert);\
void Handle##REQUEST(TcpConnectorPtr conn,const REQUEST& request)\

#define REPLY_PROTO_MSG(REQUEST,RESPONSE)\
uint32_t Reply##REQUEST(TcpConnectorPtr conn,const REQUEST& request,RESPONSE& response);\
template<>\
ProtoCallbackReply<REQUEST,RESPONSE> ProtoCallbackReply<REQUEST,RESPONSE>::Instance(Reply##REQUEST,NoConvert);\
uint32_t Reply##REQUEST(TcpConnectorPtr conn,const REQUEST& request,RESPONSE& response)

int main()
{
    GreetInfo greet;
    greet.set_msg("hello");
    greet.set_from("zhangyile");
    greet.set_to("duyankai");

    std::shared_ptr<Packet> packetPtr(new Packet());
    packetPtr->set_name(GreetInfo::descriptor()->full_name());
    packetPtr->set_content(greet.SerializeAsString());
    packetPtr->set_seq(0);
    packetPtr->set_timestamp(0);
    packetPtr->set_code(0);

    TcpConnectorPtr conn;
    ProtobufDispatcher::GetInstance()->onProtobufMessage(conn,packetPtr);

    GreetIN in;
    in.mutable_info()->CopyFrom(greet);
    std::shared_ptr<Packet> packetPtr2(new Packet());
    packetPtr2->set_name(GreetIN::descriptor()->full_name());
    packetPtr2->set_content(in.SerializeAsString());
    packetPtr2->set_seq(0);
    packetPtr2->set_timestamp(0);
    packetPtr2->set_code(0);
    ProtobufDispatcher::GetInstance()->onProtobufMessage(conn,packetPtr2);
}

HANDLE_PROTO_MSG(GreetInfo)
{
	printf("HANDLE_PROTO_MSG GreetInfo\n");
}

REPLY_PROTO_MSG(GreetIN,GreetOUT)
{
	printf("REPLY_PROTO_MSG GreetIN,GreetOUT\n");
	return 0;
}
