#pragma once
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include "Const.h"
#include <queue>
#include <mutex>
#include <iostream>
#include "MsgNode.h"
using boost::asio::ip::tcp;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::asio::awaitable;
using boost::asio::strand;
using boost::asio::io_context;
namespace this_coro = boost::asio::this_coro;

class CServer;
class CSession:public std::enable_shared_from_this<CSession>
{
public:
    ~CSession();
    CSession(boost::asio::io_context& io_context,CServer* server);
    boost::asio::ip::tcp::socket& GetSocket();
    std::string& GetUuid();
    void Start();
    void Close();
    void Send(const char* msg,short msg_length,short msg_id);
    void Send(std::string msg,short msg_id);
private:
    void HandleWrite(const boost::system::error_code& error,std::shared_ptr<CSession> self_session);
    boost::asio::io_context& _io_context;
    CServer* _server;
    boost::asio::ip::tcp::socket _socket;
    std::string _uuid;
    bool _b_close;
    std::mutex _send_lock;
    std::queue<std::shared_ptr<SendNode>> _send_que;
    std::shared_ptr<RecvNode> _recv_msg_node;
    std::shared_ptr<MsgNode> _recv_head_node;
};

class LogicNode
{
public:
    LogicNode(std::shared_ptr<CSession>,std::shared_ptr<RecvNode>);
    std::shared_ptr<CSession> _session;
    std::shared_ptr<RecvNode> _recvNode;
};