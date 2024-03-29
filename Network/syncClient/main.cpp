#include <boost/asio.hpp>
#include <iostream>
using namespace boost::asio::ip;
using namespace std;
//收发最大长度
constexpr int MAX_LENGTH = 1024;

int main(){

    try
    {
        //创建下上文服务
        boost::asio::io_context ioc;
        //构造 endpoint
        tcp::endpoint remote_ep(address::from_string("127.0.0.1"),10086);
        tcp::socket socket(ioc);
        boost::system::error_code error = boost::asio::error::host_not_found;
        socket.connect(remote_ep,error);
        if(error){
            cout<<"connect failed,code = "<<error.value()<<" . Message = "<<error.what()<<endl;
            return 0;
        }
        cout<<"Enter message:";
        char request[MAX_LENGTH] = {0};

        std::cin.getline(request,MAX_LENGTH-1);
        size_t request_length = strlen(request);
        boost::asio::write(socket,boost::asio::buffer(request,request_length));

        char reply[MAX_LENGTH] = {0};
        size_t reply_length = boost::asio::read(socket,boost::asio::buffer(reply,request_length));
        std::cout<<"Reply is :";
        std::cout.write(reply,reply_length);
        std::cout<<std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr <<"Exception : "<< e.what() << '\n';
    }
    
    system("pause");
    return 0;
}