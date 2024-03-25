#include "../inc/CSession.h"
#include "../inc/CServer.h"
#include "../inc/LogicSystem.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
CSession::CSession(boost::asio::io_context& io_context,CServer* server):_io_context(io_context),_server(server),_socket(io_context),_uuid(boost::uuids::to_string(boost::uuids::random_generator()()))
,_b_close(false),_recv_head_node(std::make_shared<MsgNode>(HEAD_TOTAL))
{

}

boost::asio::ip::tcp::socket& CSession::GetSocket()
{
    return _socket;
}

std::string& CSession::GetUuid()
{
    return _uuid;
}

void CSession::Start()
{
    //伪闭关
    auto shared_this = shared_from_this();
    //使用协程接收数据
    boost::asio::co_spawn(_io_context,[=]()->boost::asio::awaitable<void>
    {
        try
        {
            /* code */
            for(;;this->_b_close)
            {
                this->_recv_head_node->Clear();
                std::size_t n = co_await boost::asio::async_read(this->_socket,boost::asio::buffer(_recv_head_node->_data,HEAD_TOTAL),use_awaitable);

                if(n == 0)
                {
                    std::cout<<"receive peer closed"<<std::endl;
                    Close();
                    _server->ClearSession(this->_uuid);
                    co_return;
                }

                //获取头部信息
                short msg_id = 0;
                memcpy(&msg_id,_recv_head_node->_data,HEAD_ID_LEN);
                msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);                
                if(msg_id > MAX_LENGTH)
                {
                    std::cout<<"invalid msg id is "<<msg_id<<std::endl;
                    Close();
                    _server->ClearSession(_uuid);
                    co_return;
                }

                short msg_length = 0;
                memcpy(&msg_length,_recv_head_node->_data+HEAD_ID_LEN,HEAD_DATA_LEN);
                msg_length = boost::asio::detail::socket_ops::network_to_host_short(msg_length);
                if(msg_length > MAX_LENGTH)
                {
                    std::cout<<"invalid msg length is "<<msg_length<<std::endl;
                    Close();
                    _server->ClearSession(_uuid);
                    co_return;
                }

                _recv_msg_node = std::make_shared<RecvNode>(msg_length,msg_id);
                //读取包体
                n = co_await boost::asio::async_read(_socket,boost::asio::buffer(_recv_msg_node->_data,_recv_msg_node->_total_len),use_awaitable);
                if(n == 0)
                {
                    std::cout<<"receive peer closed "<<std::endl;
                    Close();
                    _server->ClearSession(_uuid);
                    co_return;
                }

                _recv_msg_node->_data[_recv_msg_node->_total_len] = 0;
                std::cout<<"receive data is "<<std::string(_recv_msg_node->_data,_recv_msg_node->_total_len)<<std::endl;
                //投递逻辑线程处理
                LogicSystem::GetInstance().PostMsgToQue(std::make_shared<LogicNode>(shared_from_this(),_recv_msg_node));
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << "Exception: "<<e.what() << '\n';
            this->Close();
            this->_server->ClearSession(this->_uuid);
        }
        
    },
    boost::asio::detached
    );

}

void CSession::Close()
{
    _socket.close();
    _b_close = true;
}

CSession::~CSession()
{
    try
    {
        /* code */
        std::cout<<"~CSession destruct"<<std::endl;
        Close();
        _server->ClearSession(_uuid);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception is "<<e.what() << '\n';
    }
    
}

void CSession::Send(const char* msg,short msg_length,short msg_id)
{
    std::unique_lock<std::mutex> lock(_send_lock);
    int send_que_size = _send_que.size();
    if(send_que_size > MAX_SENDQUE)
    {
        std::cout<<"session : "<<_uuid<<" send que fulled,size is "<<MAX_SENDQUE<<std::endl;
        return;
    }   

    if(_send_que.size()>0)
    {
        _send_que.push(std::make_shared<SendNode>(msg,msg_length,msg_id));
        return;
    }
    _send_que.push(std::make_shared<SendNode>(msg,msg_length,msg_id));

    auto msgnode = _send_que.front();
    lock.unlock();
    boost::asio::async_write(_socket,boost::asio::buffer(msgnode->_data,msgnode->_total_len),
    std::bind(&CSession::HandleWrite,this,std::placeholders::_1,shared_from_this()));
}

void CSession::Send(std::string msg,short msg_id)
{
    Send(msg.c_str(),msg.length(),msg_id);
}

void CSession::HandleWrite(const boost::system::error_code& error,std::shared_ptr<CSession> self_session)
{
    try
    {
        /* code */
        if(!error)
        {
            std::unique_lock<std::mutex> lock(_send_lock);
            _send_que.pop();
            if(!_send_que.empty())
            {
                auto msgnode = _send_que.front();
                lock.unlock();
                boost::asio::async_write(_socket,boost::asio::buffer(msgnode->_data,msgnode->_total_len),
                std::bind(&CSession::HandleWrite,this,std::placeholders::_1,shared_from_this()));
            }
        }
        else
        {
            std::cout<<"handle write failed,error is "<<error.what()<<std::endl;
            Close();
            _server->ClearSession(_uuid);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr<<"Exception is " << e.what() << '\n';
        Close();
        _server->ClearSession(_uuid);
    }   
}

LogicNode::LogicNode(std::shared_ptr<CSession> session,std::shared_ptr<RecvNode>recvNode)
:_session(session),
_recvNode(recvNode)
{

}