#pragma once
#include<map>
#include <memory>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include "CSession.h"
using boost::asio::ip::tcp;
class CServer{
public:
    CServer(boost::asio::io_context& ioc,short port);
    void ClearSession(std::string uuid);
private:
    void start_accept();
    void handle_accept(std::shared_ptr<CSession> new_session,const boost::system::error_code& error);
    boost::asio::io_context& _ioc;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::map<std::string,std::shared_ptr<CSession>> _sessions;
};