#include "endpoint.h"
#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include<vector>
#include<memory>
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

int create_accepctor_socket()
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

int bind_acceptor_socket()
{
    unsigned short port = 3333;
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(),port);
    boost::asio::io_context ios;
    boost::asio::ip::tcp::acceptor acceptor(ios,ep.protocol());
    boost::system::error_code ec;
    acceptor.bind(ep,ec);
    if(ec.value())
    {
        std::cout<<"Failed to bind the acceptor socket. Error code = "
        <<ec.value()<<". Message = "<<ec.message()<<std::endl;
        return ec.value();
    }




    return 0;
}

int connect_to_end()
{
    std::string raw_ip_address = "127.0.0.0";
    unsigned short port = 3333;
    try
    {
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(raw_ip_address),port);
        boost::asio::io_context ios;
        boost::asio::ip::tcp::socket socket(ios,ep.protocol());
        socket.connect(ep);
    }
    catch(boost::system::system_error& e)
    {
        std::cout<<"Error occured! Error code = "<<e.code()
        <<".Message = "<<e.what()<<std::endl;

        return e.code().value();
    }
    

    return 0;
}
int dns_connect_to_end()
{
    std::string host= "www.baidu.com";
    std::string port = "3333";
    boost::asio::io_context ios;
    boost::asio::ip::tcp::resolver::query resolver_qurey(host,port,boost::asio::ip::tcp::resolver::query::numeric_service);
    boost::asio::ip::tcp::resolver resolver(ios);

    try
    {
        boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(resolver_qurey);
        boost::asio::ip::tcp::socket socket(ios);
        boost::asio::connect(socket,it);
    }
    catch(boost::system::system_error& e)
    {
        std::cout<<"Error occured! Error code = "<<e.code()
        <<".Message = "<<e.what()<<std::endl;

        return e.code().value();
    }
    
}

int accept_new_connection()
{
    constexpr int BACKLOG_SIZE = 30;//
    unsigned short port = 3333;
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(),port);
    boost::asio::io_context ios;
    try
    {
        boost::asio::ip::tcp::acceptor acceptor(ios,ep.protocol());
        acceptor.bind(ep);
        acceptor.listen(BACKLOG_SIZE);
        boost::asio::ip::tcp::socket socket(ios);
        acceptor.accept(socket);
    }
    catch(boost::system::system_error& e)
    {
        std::cerr << e.what() << '\n';
        return e.code().value();
    }
    

    return 0;

}

void use_const_buffer()
{
    std::string buf("hello world");
    boost::asio::const_buffer asio_buf(buf.c_str(),buf.length());
    std::vector<boost::asio::const_buffer> buffers_sequence;
    buffers_sequence.push_back(asio_buf);

}

void use_buffer_str()
{
    boost::asio::const_buffers_1 output_buf = boost::asio::buffer("hello world");

}

void use_buffer_array(){
    const size_t BUF_SIZE_BYTES = 20;
    //使用智能指针托管  字符数组  
    std::unique_ptr<char[]>  buf(new char[BUF_SIZE_BYTES]);
    auto input_buf = boost::asio::buffer(static_cast<void*>(buf.get()),BUF_SIZE_BYTES);
}

void write_to_socket(boost::asio::ip::tcp::socket& sock){
    std::string buf = "hello world";
    std::size_t total_bytes_written = 0;
    //循环发送
    while(total_bytes_written < buf.size()){
        total_bytes_written += sock.write_some(boost::asio::buffer(buf.c_str()+total_bytes_written,buf.size()-total_bytes_written));
    }
}