#pragma once
#include <memory>
#include <boost/asio.hpp>
#include <string>
#include <queue>
#include <iostream>
const int RECVSIZE = 1024;
class MsgNode
{
public:
    MsgNode(const char* msg,int total_len):_total_len(total_len),_cur_len(0),_msg(nullptr)
    {
        _msg = new char[_total_len]();
        memcpy(_msg,msg,_total_len);
    }

    MsgNode(int total_len):_total_len(total_len),_cur_len(0),_msg(nullptr)
    {
        _msg = new char[_total_len]();
    }

    ~MsgNode()
    {
        if(!_msg)
        {
            delete[] _msg;
            _msg = nullptr;
        }
    }
public:
    //总长度
    int _total_len;
    //当前长度
    int _cur_len;
    //消息首地址
    char* _msg;    
};

class Session{
public:
    Session(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void connect(const boost::asio::ip::tcp::endpoint& ep);
    void WriteCallbackErr(const boost::system::error_code&ec,std::size_t bytes_tranferred,
    std::shared_ptr<MsgNode>);
    void WriteToSocketErr(const std::string buf);

    void WriteCallback(const boost::system::error_code&ec,std::size_t bytes_tranferred);
    void WriteToSocket(const std::string buf);

    void WriteAllToSocket(const std::string buf);
    void WriteAllCallback(const boost::system::error_code&ec,std::size_t bytes_tranferred);

    void ReadFromSocket();
    void ReadCallback(const boost::system::error_code&ec,std::size_t bytes_tranferred);

    void ReadAllFromSocket();
    void ReadAllCallback(const boost::system::error_code&ec,std::size_t bytes_tranferred);
private:
    std::queue<std::shared_ptr<MsgNode>> _send_queue;
    std::shared_ptr<boost::asio::ip::tcp::socket> _socket;
    std::shared_ptr<MsgNode> _send_node;
    bool _send_pending;

    std::shared_ptr<MsgNode> _recv_node;
    bool _recv_pending;
};