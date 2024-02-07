#include <iostream>
#include <boost/asio.hpp>
#include "../inc/Session.h"
#include "../inc/CSession.h"
#include "../inc/CServer.h"
//echo server   不能用于生产
/*
未处理 粘包问题
二次析构
*/
int main(){
    try
    {
        boost::asio::io_context ioc;
        // Server server(ioc,10086);
        CServer server(ioc,10086);
        ioc.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}