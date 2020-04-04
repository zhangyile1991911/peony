//
// Created by zhangyile on 18-6-23.
//

#ifndef ATELIER_CALLBACK_H
#define ATELIER_CALLBACK_H

#include <memory>
#include <functional>
const int	OK = 1;
const int	FAIL = -1;
typedef std::function<void()>		Callback;
typedef std::function<int()>		ICallback;
class TcpConnector;
class Buffer;
typedef std::function<void(std::shared_ptr<TcpConnector>,Buffer&)>    MessageCallback;
typedef std::function<void(std::shared_ptr<TcpConnector>)>            ConnectorCallback;

#endif //ATELIER_CALLBACK_H
