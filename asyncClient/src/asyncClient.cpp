#include <boost/asio.hpp>
#include <iostream>
#include<thread>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "Const.h"
#include <chrono>

using boost::asio::ip::address;
using boost::asio::ip::tcp;
using std::cout;
using std::endl;

std::vector<std::thread> vec_threads;

int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    for(int i =0;i<100;i++)
    {
        vec_threads.emplace_back([](){
            try
            {
                // 创建上下文服务
                boost::asio::io_context ioc;
                // 构造endpoint
                tcp::endpoint remote_ep(address::from_string("127.0.0.1"), 10086);
                tcp::socket sock(ioc);
                boost::system::error_code error = boost::asio::error::host_not_found;
                sock.connect(remote_ep, error);
                if (error)
                {
                    cout << "connect failed, code is " << error.value() << " error msg is " << error.message();
                    return 0;
                }

                int i =0;
                while(i++ < 500)
                {
                    Json::Value root;
                    root["id"] = 1001;
                    root["data"] = "hello world";

                    std::string request = root.toStyledString();
                    size_t req_len = request.length();
                    char send_data[MAX_LENGTH] = {0};
                    
                    int msgid = 1001;
                    int msgid_net = boost::asio::detail::socket_ops::host_to_network_short(msgid);
                    memcpy(send_data,&msgid_net,2);

                    size_t req_len_net = boost::asio::detail::socket_ops::host_to_network_short(req_len);
                    memcpy(send_data+2,&req_len_net,2);

                    memcpy(send_data+4,request.c_str(),req_len);       
                    boost::asio::write(sock,boost::asio::buffer(send_data,req_len+4));
                    std::cout<<"begin to receive ..."<<std::endl;

                    char reply_head[HEAD_TOTAL_LEN] = {0};
                    size_t reply_head_length =  boost::asio::read(sock,boost::asio::buffer(reply_head,HEAD_TOTAL_LEN));
                    short reply_msgid = 0;
                    memcpy(&reply_msgid,reply_head,HEAD_ID_LEN);
                    short reply_msgid_host = boost::asio::detail::socket_ops::network_to_host_short(reply_msgid);
                    std::cout<<"msg id is "<<reply_msgid_host<<std::endl;

                    short reply_msglen = 0;
                    memcpy(&reply_msglen,reply_head + HEAD_ID_LEN, HEAD_DATA_LEN);
                    short reply_msglen_host = boost::asio::detail::socket_ops::network_to_host_short(reply_msglen);
                    std::cout<<"msg len is "<<reply_msglen_host <<std::endl;

                    char reply_msg[MAX_LENGTH] = {0};
                    size_t reply_msg_len =  boost::asio::read(sock,boost::asio::buffer(reply_msg,reply_msglen_host));
                    
                    std::cout<<"reply_msg :"<<reply_msg<<std::endl; 
                    
                }
            }
            catch (std::exception& ex){
                std::cerr<<"Exception:"<<ex.what()<<std::endl;
                return 0;
            }

            return 0;
    });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }


    for(auto& t:vec_threads)
    {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();


    auto during = std::chrono::duration_cast<std::chrono::seconds>(end-start);
    std::cout<<"Time spent "<<during.count()<<" seconds"<<std::endl;


    return 0;

    try
    {   
        // 创建上下文服务
        boost::asio::io_context ioc;
        // 构造endpoint
        tcp::endpoint remote_ep(address::from_string("127.0.0.1"), 10086);
        tcp::socket sock(ioc);
        boost::system::error_code error = boost::asio::error::host_not_found;
        sock.connect(remote_ep, error);
        if (error)
        {
            cout << "connect failed, code is " << error.value() << " error msg is " << error.message();
            return 0;
        }

        Json::Value root;
        root["id"] = 1001;
        root["data"] = "hello world";
        std::string request = root.toStyledString();
        size_t req_len = request.length();
        char send_data[MAX_LENGTH] = {0};
        
        int msgid = 1001;
        int msgid_net = boost::asio::detail::socket_ops::host_to_network_short(msgid);
        memcpy(send_data,&msgid_net,2);

        size_t req_len_net = boost::asio::detail::socket_ops::host_to_network_short(req_len);
        memcpy(send_data+2,&req_len_net,2);

        memcpy(send_data+4,request.c_str(),req_len);       
        boost::asio::write(sock,boost::asio::buffer(send_data,req_len+4));
        std::cout<<"begin to receive ..."<<std::endl;

        char reply_head[HEAD_TOTAL_LEN] = {0};
        size_t reply_head_length =  boost::asio::read(sock,boost::asio::buffer(reply_head,HEAD_TOTAL_LEN));
        short reply_msgid = 0;
        memcpy(&reply_msgid,reply_head,HEAD_ID_LEN);
        short reply_msgid_host = boost::asio::detail::socket_ops::network_to_host_short(reply_msgid);
        std::cout<<"msg id is "<<reply_msgid_host<<std::endl;

        short reply_msglen = 0;
        memcpy(&reply_msglen,reply_head + HEAD_ID_LEN, HEAD_DATA_LEN);
        short reply_msglen_host = boost::asio::detail::socket_ops::network_to_host_short(reply_msglen);
        std::cout<<"msg len is "<<reply_msglen_host <<std::endl;

        char reply_msg[MAX_LENGTH] = {0};
        size_t reply_msg_len =  boost::asio::read(sock,boost::asio::buffer(reply_msg,reply_msglen_host));
        
        std::cout<<"reply_msg :"<<reply_msg<<std::endl;

        // Json::Value root2;
        // Json::Reader reader;
        // reader.parse(std::string(reply_msg,reply_msg_len),root2);
        // std::cout<<"root2 id is "<<root2["id"]<<"  data is "<<root2["data"]<<std::endl;
        #pragma region 
        /*  
        //创建发送消息线程
        std::thread send_thread([&sock]
        {
            for(;;)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));//让出系统资源
                const char* request = "hello wolrd";
                size_t request_length = strlen(request);
                char send_data[MAX_LENGTH] ={0};
                memcpy(send_data,&request_length,2);
                memcpy(send_data + 2,request,request_length);
                boost::asio::write(sock,boost::asio::buffer(send_data,request_length + 2));
            }
        });
        //创建接收消息线程
        std::thread recv_thread([&sock]
        {
            for(;;)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));//让出系统资源
                std::cout<<"begin to receive..."<<std::endl;
                char reply_head[HEAD_LENGTH]{0};
                size_t reply_length = boost::asio::read(sock,boost::asio::buffer(reply_head,HEAD_LENGTH));
                short msglen = 0;
                memcpy(&msglen,reply_head,HEAD_LENGTH);
                char msg[MAX_LENGTH] = {0};
                size_t msg_length = boost::asio::read(sock,boost::asio::buffer(msg,msglen));
                
                std::cout<<"Reply is : ";
                std::cout.write(msg,msglen)<<std::endl;
                std::cout<<"Reply len is "<<msglen;
                std::cout<<std::endl;
            }
        }
        );

        send_thread.join();
        recv_thread.join();
*/
        
        /*  echo 客户端模式
        // 创建上下文服务
        boost::asio::io_context ioc;
        // 构造endpoint
        tcp::endpoint remote_ep(address::from_string("127.0.0.1"), 10086);
        tcp::socket sock(ioc);
        boost::system::error_code error = boost::asio::error::host_not_found;
        ;
        sock.connect(remote_ep, error);
        if (error)
        {
            cout << "connect failed, code is " << error.value() << " error msg is " << error.message();
            return 0;
        }
        std::cout << "Enter message: ";
        char request[MAX_LENGTH];
        std::cin.getline(request, MAX_LENGTH);
        size_t request_length = strlen(request);
        char send_data[MAX_LENGTH] = {0};
        memcpy(send_data, &request_length, 2);
        memcpy(send_data + 2, request, request_length);
        boost::asio::write(sock, boost::asio::buffer(send_data, request_length + 2));
        char reply_head[HEAD_LENGTH];
        size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_LENGTH));
        short msglen = 0;
        memcpy(&msglen, reply_head, HEAD_LENGTH);
        char msg[MAX_LENGTH] = {0};
        size_t msg_length = boost::asio::read(sock, boost::asio::buffer(msg, msglen));
        std::cout << "Reply is: ";
        std::cout.write(msg, msglen) << endl;
        std::cout << "Reply len is " << msglen;
        std::cout << "\n";
        */
       #pragma endregion
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << endl;
    }

    system("pause");
    return 0;
}