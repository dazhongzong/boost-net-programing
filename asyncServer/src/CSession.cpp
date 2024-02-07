#include "../inc/CSession.h"
#include "../inc/CServer.h"
void CSession::Send(char *msg, int max_length)
{
    bool pending = false;
    std::lock_guard<std::mutex> lock(_send_lock);
    if (!_send_que.empty())
    {
        pending = true;
    }
    _send_que.push(std::make_shared<MsgNode>(msg, max_length));
    if (pending)
    {
        return;
    }
    boost::asio::async_write(_socket, boost::asio::buffer(msg, max_length),
                             std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

void CSession::Start()
{
    memset(_data, 0, max_length);
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
                            std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void CSession::HandleRead(const boost::system::error_code &ec, size_t bytes_transferred, std::shared_ptr<CSession> _self_CSession)
{
    if (!ec)
    {
        // 同时挂起 读写事件
        std::cout << "server receive data is" << _data << std::endl;
        Send(_data,bytes_transferred);
        memset(_data, 0, max_length);
        _socket.async_read_some(boost::asio::buffer(_data, max_length),
                                std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_CSession));
    }
    else
    {
        std::cout << "read error " << ec.value() << std::endl;
        // delete this;
        _server->ClearSession(_uuid);
    }
}
void CSession::HandleWrite(const boost::system::error_code &ec, std::shared_ptr<CSession> _self_CSession)
{
    if (!ec)
    {
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_que.pop();
        if (!_send_que.empty())
        {
            auto &msgnode = _send_que.front();
            boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_msg, msgnode->_total_len),
                                     std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
        }
    }
    else
    {
        std::cout << "write error " << ec.value() << std::endl;
        // delete this;//有隐患
        _server->ClearSession(_uuid);
    }
}

std::string &CSession::GetUuid()
{
    return _uuid;
}
