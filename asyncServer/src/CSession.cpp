#include "../inc/CSession.h"
#include "../inc/CServer.h"
#include <sstream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
void CSession::Send(char *msg, int length,short msg_id)
{
    bool pending = false;
    std::lock_guard<std::mutex> lock(_send_lock);
    int send_que_size = _send_que.size();
    if(send_que_size > MAX_SENDQUE)
    {
        std::cout<<"session:"<<_uuid<<"send que fulled size is"<<MAX_SENDQUE <<std::endl;
        return;
    }
    if (!_send_que.empty())
    {
        pending = true;
    }
    _send_que.push(std::make_shared<SendNode>(msg, length,msg_id));
    if (pending)
    {
        return;
    }
    auto & msgnode = _send_que.front();
    boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
                             std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

void CSession::Send(std::string msg,short msg_id)
{
    Send(&msg[0],msg.length(),msg_id);
}

void CSession::Close()
{
    _socket.close();
    _b_close = true;
}

void CSession::Start()
{
    // _recv_head_node->Clear();
    // boost::asio::async_read(_socket,boost::asio::buffer(_recv_head_node->_data,HEAD_TOTAL_LEN),
    // std::bind(&CSession::HandleReadHead,this,std::placeholders::_1,std::placeholders::_2,shared_from_this()));
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
        // 验证是否存在粘包
        // PrintRecvData(_data,bytes_transferred);
        // std::chrono::milliseconds dura(2000);
        // std::this_thread::sleep_for(dura);
        // 已经移动的字节数
        int copy_len = 0;
        while (bytes_transferred)
        {
            
            // 解析头部
            if (!_b_head_parse)
            {
                if (bytes_transferred + _recv_head_node->_cur_len < HEAD_TOTAL_LEN)
                {
                    memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_head_node->_cur_len += bytes_transferred;
                    memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                            std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_CSession));
                    return;
                }


                int head_remain = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
                memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
                //更新已处理的长度
                copy_len += head_remain;
                //更新未处理的长度
                bytes_transferred -= head_remain;
                //解析头部数据
                short msg_id = 0;
                memcpy(&msg_id,_recv_head_node->_data,HEAD_ID_LEN);
                msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
                std::cout<<"msg id is "<< msg_id<<std::endl;
                //id
                if(msg_id >  MAX_LENGTH)
                {
                    std::cout<<"invalid msg_id is "<<msg_id<<std::endl;
                    _server->ClearSession(_uuid);
                    return ;
                }

                short data_len = 0;
                memcpy(&data_len,_recv_head_node->_data + HEAD_ID_LEN,HEAD_DATA_LEN);
                //网络字节序转换成本地字节序
                data_len = boost::asio::detail::socket_ops::network_to_host_short(data_len);
                std::cout<<"data_len is "<<data_len <<std::endl;
                //判断消息体长度
                if(data_len > MAX_LENGTH)
                {
                    std::cout<<"invalid data_len "<<data_len<<std::endl;
                    _server->ClearSession(_uuid);
                    return ;
                }
                //右值 调用移动构造赋值语句  将会释放掉原本的空间
                _recv_msg_node = std::make_shared<RecvNode>(data_len,msg_id);
                if(bytes_transferred<data_len)
                {
                    memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len,_data + copy_len,bytes_transferred);
                    _recv_msg_node->_cur_len += bytes_transferred;
                    memset(_data,0,MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data,MAX_LENGTH),
                                            std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_CSession));
                    //头部处理完成
                    _b_head_parse = true;
                    return ; 
                }

                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len,_data + copy_len,data_len);
                _recv_msg_node->_cur_len += data_len;
                copy_len +=data_len;
                bytes_transferred -=data_len;
                _recv_msg_node->_data[_recv_msg_node->_total_len] = 0;
                std::cout<<"receive data is "<<_recv_msg_node->_data <<std::endl;
                //调用Send发送测试
                Json::Value root;
                Json::Reader reader;
                reader.parse(_recv_msg_node->_data,root);
                Send(root.toStyledString(),root["id"].asInt());
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
                memcpy(_recv_msg_node->_data+_recv_msg_node->_cur_len,_data + copy_len,bytes_transferred);
                memset(_data,0,MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data,MAX_LENGTH),
                std::bind(&CSession::HandleRead,this,std::placeholders::_1,std::placeholders::_2,_self_CSession));
                return ;
            }
            //消息体部分处理完全
            memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len,_data + copy_len,remain_msg);
            _recv_msg_node->_cur_len += remain_msg;
            bytes_transferred -= remain_msg;
            _recv_msg_node->_data[_recv_msg_node->_total_len] = 0;
            std::cout<<"receive body data is : "<<_recv_msg_node->_data<<std::endl;

            Json::Value root;
            Json::Reader reader;
            reader.parse(_recv_msg_node->_data,root);
            Send(root.toStyledString(),root["id"].asInt());
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
            boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
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


void CSession::HandleReadHead(const boost::system::error_code& ec,size_t bytes_transferred,std::shared_ptr<CSession> _self_CSession)
{
    if(!ec)
    {
        if(bytes_transferred < HEAD_TOTAL_LEN)
        {
            std::cout<<"read head length error";
            Close();
            _server->ClearSession(_uuid);
            return ;
        }

        //头部接收完全，开始解析头部
        short msg_id = 0;
        memcpy(&msg_id,_recv_head_node->_data,HEAD_ID_LEN);
        msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);

        short data_len = 0;
        memcpy(&data_len,_recv_head_node->_data+HEAD_ID_LEN,HEAD_DATA_LEN);
        data_len = boost::asio::detail::socket_ops::network_to_host_short(data_len);
        std::cout<<"data len is "<<data_len <<std::endl;
    
        if(data_len >MAX_LENGTH)
        {
            std::cout<<"invalid data length is "<< data_len <<std::endl;
            _server->ClearSession(_uuid);
            return;
        }

        _recv_msg_node = std::make_shared<RecvNode>(data_len,msg_id);
        boost::asio::async_read(_socket,boost::asio::buffer(_recv_msg_node->_data,data_len),
            std::bind(&CSession::HandleReadMsg,this,std::placeholders::_1,std::placeholders::_2,shared_from_this())
        );
    }
    else 
    {
        std::cout<<"handle read head failed, error is "<<ec.what()<<std::endl;
        Close();
        _server->ClearSession(_uuid);
    }
}

void CSession::HandleReadMsg(const boost::system::error_code& ec,size_t bytes_transferred,std::shared_ptr<CSession> _self_CSession)
{
    if(!ec)
    {
        // PrintRecvData(_recv_msg_node->_data,bytes_transferred);
        _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
    

        std::cout<<"receive data is "<<_recv_msg_node->_data<<std::endl;
        Send(_recv_msg_node->_data,_recv_msg_node->_total_len,_recv_msg_node->_msg_id);
        _recv_head_node->Clear();

        boost::asio::async_read(_socket,boost::asio::buffer(_recv_head_node->_data,HEAD_TOTAL_LEN),
        std::bind(&CSession::HandleReadHead,this,std::placeholders::_1,std::placeholders::_2,shared_from_this()));
    }
    else 
    {
        std::cout<<"handle read msg failed, error is "<<ec.what()<<std::endl;
        Close();
        _server->ClearSession(_uuid);
    }
}