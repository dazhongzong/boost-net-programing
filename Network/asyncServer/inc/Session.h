#pragma once
#include<map>
#include <memory>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
using boost::asio::ip::tcp;
class Server;
class Session: public std::enable_shared_from_this<Session>
{
private:
    boost::asio::ip::tcp::socket _socket;
    enum {max_length = 1024};
    char _data[max_length]; 
    Server* _server;
    std::string _uuid;
public:
    Session(boost::asio::io_context& ioc,Server* server):_socket(ioc),_data{0},_server(server){
        //雪花算法  
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        _uuid = boost::uuids::to_string(a_uuid);
    }
    tcp::socket& Socket(){
        return _socket;
    }
    void Start();
    ~Session(){

    };
    std::string& GetUuid();
private:
    void handle_read(const boost::system::error_code& ec,size_t bytes_transferred,std::shared_ptr<Session> _self_session);
    void handle_write(const boost::system::error_code&ec,std::shared_ptr<Session> _self_session);
};

class Server{
public:
    Server(boost::asio::io_context& ioc,short port);
    void ClearSession(std::string uuid);
private:
    void start_accept();
    void handle_accept(std::shared_ptr<Session> new_session,const boost::system::error_code& error);
    boost::asio::io_context& _ioc;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::map<std::string,std::shared_ptr<Session>> _sessions;
};