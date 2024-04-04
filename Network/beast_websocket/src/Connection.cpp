#include "../inc/Connection.h"
#include "../inc/ConnectionMgr.h"

Connection::Connection(net::io_context& ioc)
:_ioc(ioc),_ws_ptr(std::make_unique<stream<tcp_stream>>(make_strand(ioc)))
,_uuid(boost::uuids::to_string(boost::uuids::random_generator()()))
{

}

std::string Connection::GetUuid()
{
    return _uuid;
}

net::ip::tcp::socket& Connection::GetSocket()
{
    return boost::beast::get_lowest_layer(*_ws_ptr).socket();
}

void Connection::AsyncAccept()
{
    auto self = shared_from_this();
    _ws_ptr->async_accept([self](boost::system::error_code ec)
    {
        try
        {
            /* code */
            if(!ec)
            {
                self->Start();
                ConnectionMgr::GetInstance().AddConnection(self);
            }
            else 
            {
                std::cout<<"websocket accept faild,error is "<<ec.what()<<std::endl;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr<<"websocket async accept exception is " << e.what() << '\n';
        }
        
    });
}

void Connection::Start()
{
    auto self = shared_from_this();
    _ws_ptr->async_read(_recv_buffer,[self](boost::system::error_code err,std::size_t buufer_bytes)
    {
        try
        {
            /* code */
            if(!err)
            {
                self->_ws_ptr->text(self->_ws_ptr->got_text());
                std::string recv_data = boost::beast::buffers_to_string(self->_recv_buffer.data());
                self->_recv_buffer.consume(self->_recv_buffer.size());
                std::cout<<"websocket receive msg is "<<recv_data<<std::endl;

                self->AsyncSend(std::move(recv_data));
                self->Start();
            }
            else 
            {
                std::cout <<"websocket async read error is "<<err.what()<<std::endl;
                ConnectionMgr::GetInstance().RemoveConnection(self->GetUuid());
                return ;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr <<"exception is "<< e.what() << '\n';
            ConnectionMgr::GetInstance().RemoveConnection(self->GetUuid());
        }
        
    });
}

void Connection::AsyncSend(std::string msg)
{
    {
        std::lock_guard<std::mutex> lock(_send_mutex);
        int que_len = _send_que.size();
        _send_que.emplace(msg);
        if(que_len >0)
        {
            return ;
        }
    }

    auto self = shared_from_this();
    _ws_ptr->async_write(boost::asio::buffer(msg.c_str(),msg.length()),
    [self](boost::system::error_code err,std::size_t nsize)
    {
        try
        {
            /* code */
            if(err)
            {
                std::cout<<"async_send err is "<<err.what()<<std::endl;
                ConnectionMgr::GetInstance().RemoveConnection(self->GetUuid());
                return;
            }

            std::string send_msg;
            {
                std::lock_guard<std::mutex> lock(self->_send_mutex);
                self->_send_que.pop();
                if(self->_send_que.empty())
                {
                    return;
                }
                send_msg = self->_send_que.front();
            }
            self->AsyncSend(std::move(send_msg));
        }
        catch(const std::exception& e)
        {
            std::cerr<<"async send " << e.what() << '\n';
        }
        


    });
}