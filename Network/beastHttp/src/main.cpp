#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp       = boost::asio::ip::tcp;

namespace my_program_state
{
    //统计请求的次数
    std::size_t request_count()
    {
        static std::size_t count = 0;
        return ++count;
    }
    
    std::time_t now()
    {
        return std::time(0);
    }
}

class Http_Connection : public std::enable_shared_from_this<Http_Connection>
{
public:
    Http_Connection(tcp::socket socket):_socket(std::move(socket))
    {

    }

    void Start()
    {
        //读请求
        Read_Request();
        //超时检测
        Check_deadline();

    }

    void Stop()
    {

    }
private:
    tcp::socket _socket;
    beast::flat_buffer _buffer{8192};
    http::request<http::dynamic_body> _request;
    http::response<http::dynamic_body> _response;

    net::steady_timer _deadline{
        _socket.get_executor(),std::chrono::seconds(60)
    };

    void Read_Request()
    {
        auto self = shared_from_this();
        http::async_read(_socket,_buffer,_request,[self](beast::error_code ec,std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);
            if(!ec)
            {
                self->Proccess_request();   
            }
        });
    }

    void Check_deadline()
    {
        auto self = shared_from_this();

        _deadline.async_wait([self](boost::system::error_code ec){
            if(!ec)
            {
                self->_socket.close(ec);
            }
            else
            {

            }
        });
    }

    void Proccess_request()
    {
        _response.version(_request.version());
        _response.keep_alive(false);
        
        switch(_request.method())
        {
        case http::verb::get:
            _response.result(http::status::ok);
            _response.set(http::field::server,"Beast");
            Create_Response();
            break;
        case http::verb::post:
            _response.result(http::status::ok);
            _response.set(http::field::server,"Beast");
            Create_post_response();
            break;
        default:
            _response.result(http::status::bad_request);
            _response.set(http::field::content_type,"text/plain");
            beast::ostream(_response.body())<<"invalid request-method '"<< std::string(_request.method_string())<<"'";
            break;
        }

        Write_response();
    }

    void Create_Response()
    {
        if(_request.target() == "/count")
        {
            _response.set(http::field::content_type,"text/html");
            beast::ostream(_response.body())<<"<html>\n"
            <<"<head><title>Request count</title></head>\n"
            <<"<body>\n"
            <<"<h1>Request count</h1>\n"
            <<"<p>There have been"
            <<my_program_state::request_count()
            <<" requests so far.</p>\n"
            <<"</body>\n"
            <<"</html>\n";
        }
        else if(_request.target() == "/time")
        {
            _response.set(http::field::content_type,"text/html");
            beast::ostream(_response.body())<<"<html>\n"
            <<"<head><title>Current time</title></head>\n"
            <<"<body>\n"
            <<"<h1>Current time</h1>\n"
            <<"<p>The current time is "
            <<my_program_state::now()
            <<" seconds since the epoch.</p>\n"
            <<"</body>\n"
            <<"</html>\n";
        }
        else 
        {
            _response.result(http::status::not_found);
            _response.set(http::field::content_type,"text/plain");
            beast::ostream(_response.body())<<"File not found\r\n";
        }
    }

    void Write_response()
    {
        auto self = shared_from_this();
        _response.content_length(_response.body().size());
        http::async_write(_socket,_response,[self](beast::error_code ec,std::size_t){
            self->_socket.shutdown(tcp::socket::shutdown_send);//关闭发送端         服务器不要主动关闭，因为会存在 established -> close_wait的过渡状态
            self->_deadline.cancel();
        });
    }

    void Create_post_response()
    {
        if(_request.target() == "/email")
        {
            auto &body = this->_request.body();
            auto body_str = boost::beast::buffers_to_string(body.data());
            std::cout<<"receive body is "<<body_str<<std::endl;
            this->_response.set(http::field::content_type,"text/json");
            Json::Value root;
            Json::Reader reader;
            Json::Value src_root;
            bool parse_success = reader.parse(body_str,src_root);
            if(!parse_success)
            {
                std::cout<<"Failed to parse json data"<<std::endl;
                root["error"] = 1001;
                std::string jsonstr = root.toStyledString();
                beast::ostream(_response.body())<< jsonstr;
            }

            auto email = src_root["email"].asString();
            std::cout<<"email is "<<email<<std::endl;
            root["error"] = 0;
            root["email"] = src_root["email"];
            root["msg"] = "receive email post success";
            std::string jsonstr = root.toStyledString();
            beast::ostream(_response.body())<<jsonstr;
        }
        else 
        {
            _response.result(http::status::not_found);
            _response.set(http::field::content_type,"text/plain");
            beast::ostream(_response.body())<<"File not found\r\n";
        }
    }

};

void Http_server(tcp::acceptor& acceptor,tcp::socket& socket)
{
    acceptor.async_accept(socket,[&](boost::system::error_code ec){
        if(!ec)
        {
            std::make_shared<Http_Connection>(std::move(socket))->Start();
        }

        Http_server(acceptor,socket);
    });
}

int main()
{
    try
    {
        /* code */
        auto const address = net::ip::make_address("127.0.0.1");
        unsigned short port = static_cast<unsigned short>(10086);
        net::io_context ioc{1};
        tcp::acceptor acceptor{ioc,{address,port}};
        tcp::socket socket(ioc);
        Http_server(acceptor,socket);
        ioc.run();
    }
    catch(const std::exception& e)
    {
        std::cerr<<"Exception is " << e.what() << '\n';
        return EXIT_FAILURE;
    }
    


    return 0;
}