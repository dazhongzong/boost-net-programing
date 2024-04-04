#pragma once
#include "Const.h"
#include "MsgNode.h"

#include<map>
#include <memory>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <mutex>
#include <queue>
#include <iomanip>

using boost::asio::ip::tcp;

class CServer;
class LogicSystem;

class CSession: public std::enable_shared_from_this<CSession>
{
private:
    boost::asio::ip::tcp::socket _socket;
    enum {max_length = 1024};
    char _data[MAX_LENGTH]; 
    CServer* _server;
    std::string _uuid;
    std::queue<std::shared_ptr<SendNode>> _send_que;    
    std::mutex _send_lock;
    //收到的消息体结构
    std::shared_ptr<RecvNode> _recv_msg_node;
    bool _b_head_parse;
    //收到的头部结构
    std::shared_ptr<MsgNode> _recv_head_node;
    boost::asio::strand<boost::asio::io_context::executor_type> _strand;
    bool _b_close ;
public:
    CSession(boost::asio::io_context& ioc,CServer* server):_socket(ioc),_data{0},_server(server),_b_head_parse(false),_b_close(false),_recv_head_node(new MsgNode(HEAD_TOTAL_LEN)),
    _strand(ioc.get_executor())
    {
        //雪花算法  
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        _uuid = boost::uuids::to_string(a_uuid);
    }
    tcp::socket& Socket(){
        return _socket;
    }
    void Send(char* msg,int max_length,short msg_id);
    void Send(std::string msg,short msg_id);
    void Start();
    std::string& GetUuid();
    void Close();
private:
    void HandleRead(const boost::system::error_code& ec,size_t bytes_transferred,std::shared_ptr<CSession> _self_CSession);
    void HandleWrite(const boost::system::error_code&ec,std::shared_ptr<CSession> _self_CSession);
    void PrintRecvData(char* data,int length);


    void HandleReadHead(const boost::system::error_code& ec,size_t bytes_transferred,std::shared_ptr<CSession> _self_CSession);
    void HandleReadMsg(const boost::system::error_code& ec,size_t bytes_transferred,std::shared_ptr<CSession> _self_CSession);
};


class LogicNode
{
    friend class LogicSystem;
public:
    LogicNode(std::shared_ptr<CSession>,std::shared_ptr<RecvNode>);
private:
    std::shared_ptr<CSession> _session;
    std::shared_ptr<RecvNode> _recvnode;
};