#include "endpoint.h"
#include <boost/asio.hpp>
#include <string>
#include <iostream>
// using namespace boost;

int client_end_point()
{
    //对端 ip、port
    std::string raw_ip_address = "127.4.8.1";
    unsigned short port = 3333;
    //boost 错误码
    boost::system::error_code ec;
    boost::asio::ip::address ip_address = boost::asio::ip::address::from_string(raw_ip_address,ec);
    if(ec.value())
    {
        std::cout<<"Failed to parse the IP address. Error code = "<<ec.value()<<" . Message is "<<ec.message()<<std::endl;
        return ec.value();
    }
    // 生成 endpoint  调用有参构造函数
    boost::asio::ip::tcp::endpoint ep(ip_address,port);
    return 0;
}

int server_end_point()
{
    //服务器本端  ip、端口号
    unsigned short port = 3333;
    boost::asio::ip::address ip_address = boost::asio::ip::address_v4::any();
    //创建服务器endpoint
    boost::asio::ip::tcp::endpoint ep(ip_address,port);
}

int create_tcp_socket()
{
    //通信所需的上下文
    boost::asio::io_context ioc;
    //通信协议  ipv4
    boost::asio::ip::tcp protocol = boost::asio::ip::tcp::v4();
    //创建 socket
    boost::asio::ip::tcp::socket socket(ioc);
    //boost error code
    boost::system::error_code ec;
    socket.open(protocol,ec);
    if(ec.value())
    {
        std::cout<<"Failed to Open socket . Error code = "<<ec.value()<<" . Message is "<<ec.message()<<std::endl;
        return ec.value();
    }
    
}

int create_accpector_socket()
{
    boost::asio::io_context ios;
    /*  旧版创建 acceptor
    boost::asio::ip::tcp::acceptor acceptor(ios);
    boost::system::error_code ec;
    boost::asio::ip::tcp protocol = boost::asio::ip::tcp::v4();
    acceptor.open(protocol,ec);
    */
    //新版                  接收ipv4 连接。
    boost::asio::ip::tcp::acceptor acceptor(ios,boost::asio::ip::tcp::v4(),3333);
}

int bind_accpetor_socket()
{
    unsigned short port = 3333;
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(),port);
}