#include "../inc/Session.h"

void Session::Start(){
    memset(_data,0,max_length);
    _socket.async_read_some(boost::asio::buffer(_data,max_length),
    std::bind(&Session::handle_read,this,std::placeholders::_1,std::placeholders::_2,shared_from_this()));
}

void Session::handle_read(const boost::system::error_code& ec,size_t bytes_transferred,std::shared_ptr<Session> _self_session){
    if(!ec){
        //同时挂起 读写事件
        std::cout<<"server receive data is"<<_data<<std::endl;
        memset(_data,0,max_length);
        _socket.async_read_some(boost::asio::buffer(_data,max_length),
        std::bind(&Session::handle_read,this,std::placeholders::_1,std::placeholders::_2,_self_session));

        boost::asio::async_write(_socket,boost::asio::buffer("hello client"),
        std::bind(&Session::handle_write,this,std::placeholders::_1,_self_session));
    
    }else{
        std::cout<<"read error "<<ec.value()<<std::endl;
        //delete this;
        _server->ClearSession(_uuid);

    }
}
void Session::handle_write(const boost::system::error_code&ec,std::shared_ptr<Session> _self_session){
    if(!ec){
        memset(_data,0,max_length);
        _socket.async_read_some(boost::asio::buffer(_data,max_length),
        std::bind(&Session::handle_read,this,std::placeholders::_1,std::placeholders::_2,_self_session));
    }else{
        std::cout<<"write error "<<ec.value()<<std::endl;
        // delete this;//有隐患
        _server->ClearSession(_uuid);
    }
}

std::string& Session::GetUuid(){
    return _uuid;
}


Server::Server(boost::asio::io_context& ioc,short port):_ioc(ioc),
_acceptor(_ioc,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port)){
    std::cout<<"Server start success, on port :"<<port<<std::endl;
    start_accept();
}

void Server::start_accept(){
    auto new_session= std::make_shared<Session>(_ioc,this);
    // Session* new_session = new Session(_ioc);
    _acceptor.async_accept(new_session->Socket(),
    std::bind(&Server::handle_accept,this,new_session,std::placeholders::_1)); //std::bind 成功后，智能指针将会通过值传递的方式 调用 拷贝构造函数  将引用计数 + 1
}
void Server::handle_accept(std::shared_ptr<Session> new_session,const boost::system::error_code& error){
    if(!error){
        new_session->Start();
        _sessions.insert(std::make_pair(new_session->GetUuid(),new_session));
    }else{
        // delete new_session;
        // new_session = nullptr;
    }
    start_accept();
}
void Server::ClearSession(std::string uuid){
    _sessions.erase(uuid);
}