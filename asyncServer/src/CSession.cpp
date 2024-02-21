#include "../inc/CSession.h"
#include "../inc/CServer.h"
#include <sstream>
void CSession::Send(char *msg, int length)
{
    bool pending = false;
    std::lock_guard<std::mutex> lock(_send_lock);
    if (!_send_que.empty())
    {
        pending = true;
    }
    _send_que.push(std::make_shared<MsgNode>(msg, length));
    if (pending)
    {
        return;
    }
    boost::asio::async_write(_socket, boost::asio::buffer(msg, length),
                             std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

void CSession::Start()
{
    memset(_data, 0, MAX_LENGTH);
    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                            std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void CSession::HandleRead(const boost::system::error_code &ec, size_t bytes_transferred, std::shared_ptr<CSession> _self_CSession)
{
    /*
    if (!ec)
    {
        // 同时挂起 读写事件
        std::cout << "server receive data is" << _data << std::endl;
        Send(_data,bytes_transferred);
        memset(_data, 0, MAX_LENGTH);
        _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_CSession));
    }
    else
    {
        std::cout << "read error " << ec.value() << std::endl;
        // delete this;
        _server->ClearSession(_uuid);
    }
    */
    if (!ec)
    {
        PrintRecvData(_data,bytes_transferred);
        std::chrono::milliseconds dura(2000);
        std::this_thread::sleep_for(dura);
        // 已经移动的字节数
        int copy_len = 0;
        while (bytes_transferred)
        {
            
            // 解析头部
            if (!_b_head_parse)
            {
                if (bytes_transferred + _recv_head_node->_cur_len < HEAD_LENGTH)
                {
                    memcpy(_recv_head_node->_msg + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_head_node->_cur_len += bytes_transferred;
                    memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                            std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_CSession));
                    return;
                }
                int head_remain = HEAD_LENGTH - _recv_head_node->_cur_len;
                memcpy(_recv_head_node->_msg + _recv_head_node->_cur_len, _data + copy_len, head_remain);
                //更新已处理的长度
                copy_len += head_remain;
                //更新未处理的长度
                bytes_transferred -= head_remain;
                //解析头部数据
                short data_len = 0;
                memcpy(&data_len,_recv_head_node->_msg,HEAD_LENGTH);
                std::cout<<"data_len is "<<data_len <<std::endl;
                //判断消息体长度
                if(data_len > MAX_LENGTH)
                {
                    std::cout<<"invalid data_len "<<data_len<<std::endl;
                    _server->ClearSession(_uuid);
                    return ;
                }
                _recv_msg_node = std::make_shared<MsgNode>(data_len);

                if(bytes_transferred<data_len)
                {
                    memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len,_data + copy_len,bytes_transferred);
                    _recv_msg_node->_cur_len += bytes_transferred;
                    memset(_data,0,MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data,MAX_LENGTH),
                                            std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_CSession));
                    //头部处理完成
                    _b_head_parse = true;
                    return ; 
                }

                memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len,_data + copy_len,data_len);
                _recv_msg_node->_cur_len += data_len;
                copy_len +=data_len;
                bytes_transferred -=data_len;
                _recv_msg_node->_msg[_recv_msg_node->_total_len] = 0;
                std::cout<<"receive data is "<<_recv_msg_node->_msg <<std::endl;
                //调用Send发送测试
                Send(_recv_msg_node->_msg,_recv_msg_node->_total_len);
                //继续轮询剩余未处理数据
                _b_head_parse = false;
                _recv_head_node->Clear();
                if(bytes_transferred <= 0)
                {
                    memset(_data,0,MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data,MAX_LENGTH),
                    std::bind(&CSession::HandleRead,this,std::placeholders::_1,std::placeholders::_2,_self_CSession));
                    return ;
                }
                continue;
            }
            
            // 头部解析完成 解析消息体剩余部分
            int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
            if(bytes_transferred <remain_msg)
            {
                // 消息体部分没有接收完全
                memcpy(_recv_msg_node->_msg+_recv_msg_node->_cur_len,_data + copy_len,bytes_transferred);
                memset(_data,0,MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data,MAX_LENGTH),
                std::bind(&CSession::HandleRead,this,std::placeholders::_1,std::placeholders::_2,_self_CSession));
                return ;
            }
            //消息体部分处理完全
            memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len,_data + copy_len,remain_msg);
            _recv_msg_node->_cur_len += remain_msg;
            bytes_transferred -= remain_msg;
            _recv_msg_node->_msg[_recv_msg_node->_total_len] = 0;
            std::cout<<"receive body data is : "<<_recv_msg_node->_msg<<std::endl;
            Send(_recv_msg_node->_msg,_recv_msg_node->_total_len);
            //开启处理下一个 消息头部和消息体
            _b_head_parse = false;
            _recv_head_node->Clear();
            //可以不掉用Clear() 因为处理头部时，会为消息体重新分配一个新内存(移动赋值)
            _recv_msg_node->Clear();

            if(bytes_transferred <= 0)
            {
                memset(_data,0,MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data,MAX_LENGTH),
                std::bind(&CSession::HandleRead,this,std::placeholders::_1,std::placeholders::_2,_self_CSession));
                return ;
            }
            //如果还有 剩余数据 一定是 消息头部
            continue;
        }
    }
    else
    {
        std::cout<<"handle read failed, error is "<<ec.what()<<std::endl;
        // Clost();
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

void CSession::PrintRecvData(char* data,int length)
{
    std::stringstream ss;
    std::string result = "0x";
    for(int i = 0;i<length;i++)
    {
        std::string hexstr;
        ss<<std::hex<<std::setw(2)<<std::setfill('0')<<int(data[i])<<std::endl;
        ss>>hexstr;
        result +=hexstr;
        std::cout<<"receive raw data is : "<<data<<std::endl;
    }
}