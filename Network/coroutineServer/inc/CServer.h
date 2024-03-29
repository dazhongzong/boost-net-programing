#pragma once
#include <memory>
#include <mutex>
#include <map>
#include <boost/asio.hpp>

class CSession;

class CServer
{
public:
    CServer(boost::asio::io_context& io_context,short port);
    ~CServer();
    void ClearSession(std::string);
private:
    void HandleAccept(std::shared_ptr<CSession>,const boost::system::error_code& error);
    void StartAccept();
    boost::asio::io_context& _io_context;
    short _port;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::map<std::string,std::shared_ptr<CSession>> _sessions;
    std::mutex _mutex;
};