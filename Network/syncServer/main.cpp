#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <set>
#include <thread>
using boost::asio::ip::tcp;
constexpr int MAX_LENGTH = 1024;

using socket_ptr = std::shared_ptr<tcp::socket>;
std::set<std::shared_ptr<std::thread>> thread_set;

void session(socket_ptr sock){
    try
    {
        for(;;){
            char data[MAX_LENGTH]= {0};
            boost::system::error_code error;
            // size_t length = boost::asio::read(sock,boost::asio::buffer(data,MAX_LENGTH),error);
            size_t length = sock->read_some(boost::asio::buffer(data,MAX_LENGTH));
            if(error == boost::asio::error::eof){
                std::cout<<"connection close by peer"<<std::endl;
                break;
            }
            else if(error){
                throw boost::system::system_error(error);
            }
            std::cout<<"receive from : "<<sock->remote_endpoint().address().to_string()<<"\ncontext: "<<data<<std::endl;
            //回传到对方
            sock->send(boost::asio::buffer(data,MAX_LENGTH));
        }

    }
    catch(const std::exception& e)
    {
        std::cerr<<"Exception in thread :" << e.what() << '\n'<<std::endl;
    }
    
}

void server(boost::asio::io_context& io_context,unsigned short port){
    boost::asio::ip::tcp::acceptor a(io_context,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port));
    for(;;){
        socket_ptr socket = std::make_shared<tcp::socket>(io_context);
        a.accept(*socket);
        auto t = std::make_shared<std::thread>(session,socket);
        thread_set.insert(t);
    }
}

int main(){
    try
    {
        boost::asio::io_context ioc;
        server(ioc,10086);
        for(auto &t:thread_set){
            t->join();
        }   
    }
    catch(const std::exception& e)
    {
        std::cerr<<"Exception is :" << e.what() << '\n'<<std::endl;
    }
        

    return 0;
}