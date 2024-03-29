#include "../inc/Session.h"

Session::Session(std::shared_ptr<boost::asio::ip::tcp::socket> socket):_socket(socket),_send_pending(false),_recv_pending(false)
{

}

void Session::connect(const boost::asio::ip::tcp::endpoint& ep)
{
    if(!_socket)
    {
        return ;
    }
    _socket->connect(ep);
}

void Session::WriteCallbackErr(const boost::system::error_code&ec,std::size_t bytes_tranferred,
std::shared_ptr<MsgNode> msg_node){
    //判断是否完全发送
    if(bytes_tranferred + msg_node->_cur_len < msg_node->_total_len){
        _send_node->_cur_len +=bytes_tranferred;
        this->_socket->async_write_some(boost::asio::buffer(_send_node->_msg + _send_node->_cur_len,_send_node->_total_len - _send_node->_cur_len),
        std::bind(&Session::WriteCallbackErr,this,std::placeholders::_1,std::placeholders::_2,_send_node));
    }
}

void Session::WriteToSocketErr(const std::string buf){
    _send_node = std::make_shared<MsgNode>(buf.c_str(),buf.length());
    _socket->async_write_some(boost::asio::buffer(_send_node->_msg,_send_node->_total_len),
    std::bind(&Session::WriteCallbackErr,this,std::placeholders::_1,std::placeholders::_2,_send_node));
}

void Session::WriteCallback(const boost::system::error_code&ec,std::size_t bytes_tranferred){
    if(ec.value()){
        std::cout<<"Error ,code is "<<ec.value()<<" . Message is "<<ec.what()<<std::endl;
        return ;
    }

    auto& send_data = _send_queue.front();
    send_data->_cur_len += bytes_tranferred;
    if(send_data->_cur_len <send_data->_total_len){
        _socket->async_write_some(boost::asio::buffer(send_data->_msg + send_data->_cur_len,send_data->_total_len - send_data->_cur_len),
        std::bind(&Session::WriteCallback,this,std::placeholders::_1,std::placeholders::_2));
        return ;
    }
    _send_queue.pop();
    if(_send_queue.empty()){
        _send_pending = false;
    }else{
        auto& send_data = _send_queue.front();
        _socket->async_write_some(boost::asio::buffer(send_data->_msg + send_data->_cur_len,send_data->_total_len - send_data->_cur_len),
        std::bind(&Session::WriteCallback,this,std::placeholders::_1,std::placeholders::_2));
        return ;
    }
}

void Session::WriteToSocket(const std::string buf){
    _send_queue.emplace(new MsgNode(buf.c_str(),buf.length()));
    if(_send_pending){
        return ;
    }

    _socket->async_write_some(boost::asio::buffer(buf),
    std::bind(&Session::WriteCallback,this,std::placeholders::_1,std::placeholders::_2));
    _send_pending = true;
}

void Session::WriteAllToSocket(const std::string buf){
    _send_queue.emplace(new MsgNode(buf.c_str(),buf.length()));
    if(_send_pending){
        return ;
    }

    _socket->async_send(boost::asio::buffer(buf),
    std::bind(&Session::WriteAllCallback,this,std::placeholders::_1,std::placeholders::_2));
    _send_pending = true;   
}

void Session::WriteAllCallback(const boost::system::error_code&ec,std::size_t bytes_tranferred){
    
    if(ec.value()){
        std::cout<<"Error ,code is "<<ec.value()<<" . Message is "<<ec.what()<<std::endl;
        return ;
    }
    
    _send_queue.pop();
    if(_send_queue.empty()){
        _send_pending = false;
    }else
    {
        auto& send_data = _send_queue.front();
        auto& send_data = _send_queue.front();
        _socket->async_send(boost::asio::buffer(send_data->_msg + send_data->_cur_len,send_data->_total_len - send_data->_cur_len),
        std::bind(&Session::WriteCallback,this,std::placeholders::_1,std::placeholders::_2));
        return ;
    }

}


void Session::ReadFromSocket(){
    if(_recv_pending){
        return ;
    }

    _recv_node = std::make_shared<MsgNode>(RECVSIZE);
    _socket->async_read_some(boost::asio::buffer(_recv_node->_msg,_recv_node->_total_len),
    std::bind(&Session::ReadCallback,this,std::placeholders::_1,std::placeholders::_2));
    _recv_pending = true;
}

void Session::ReadCallback(const boost::system::error_code&ec,std::size_t bytes_tranferred){
    _recv_node->_cur_len += bytes_tranferred;
    if(_recv_node->_cur_len < _recv_node->_total_len){
        _socket->async_read_some(boost::asio::buffer(_recv_node->_msg+_recv_node->_cur_len,_recv_node->_total_len-_recv_node->_cur_len),
        std::bind(&Session::ReadCallback,this,std::placeholders::_1,std::placeholders::_2));
        return;
    }else
    {
        _recv_pending = false;
        _recv_node = nullptr;
    }
}

void Session::ReadAllFromSocket(){
    if(_recv_pending){
        return ;
    }

    _recv_node = std::make_shared<MsgNode>(RECVSIZE);
    _socket->async_receive(boost::asio::buffer(_recv_node->_msg,_recv_node->_total_len),
    std::bind(&Session::ReadAllCallback,this,std::placeholders::_1,std::placeholders::_2));
    _recv_pending = true;
}
void Session::ReadAllCallback(const boost::system::error_code&ec,std::size_t bytes_tranferred){
    _recv_node->_cur_len += bytes_tranferred;
    _recv_node = nullptr;
    _recv_pending = false;

}