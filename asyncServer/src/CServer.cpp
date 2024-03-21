#include "../inc/CServer.h"
#include "../inc/AsioIOServicePool.h"
CServer::CServer(boost::asio::io_context& ioc,short port):_ioc(ioc),
_acceptor(_ioc,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port)){
    std::cout<<"CServer start success, on port :"<<port<<std::endl;
    start_accept();
}

void CServer::start_accept(){
    auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
    auto new_session= std::make_shared<CSession>(io_context,this);      //每个连接都有自己的session
    // CSession* new_session = new CSession(_ioc);
    _acceptor.async_accept(new_session->Socket(),
    std::bind(&CServer::handle_accept,this,new_session,std::placeholders::_1)); //std::bind 成功后，智能指针将会通过值传递的方式 调用 拷贝构造函数  将引用计数 + 1
}
void CServer::handle_accept(std::shared_ptr<CSession> new_session,const boost::system::error_code& error){
    if(!error){
        new_session->Start();
        _sessions.insert(std::make_pair(new_session->GetUuid(),new_session));
    }else{
        // delete new_session;
        // new_session = nullptr;
    }
    start_accept();
}
void CServer::ClearSession(std::string uuid){
    _sessions.erase(uuid);
}