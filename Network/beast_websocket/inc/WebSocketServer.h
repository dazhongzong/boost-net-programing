#pragma once
#include "ConnectionMgr.h"
class WebSocketServer
{
public:
    WebSocketServer(const WebSocketServer&) = delete;
    WebSocketServer& operator=(const WebSocketServer&) = delete;
    WebSocketServer(net::io_context& ioc,unsigned short port);
    void StartAccept();         //tcp 层面

private:
    net::ip::tcp::acceptor _acceptor;
    net::io_context& _ioc;
};