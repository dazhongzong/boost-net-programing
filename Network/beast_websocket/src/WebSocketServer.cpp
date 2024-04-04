#include "../inc/WebSocketServer.h"

WebSocketServer::WebSocketServer(net::io_context& ioc,unsigned short port)
:_ioc(ioc),
_acceptor(ioc,net::ip::tcp::endpoint(net::ip::tcp::v4(),port))
{
    std::cout<<"Server start on port : "<<port<<std::endl;
}

void WebSocketServer::StartAccept()
{
    auto con_ptr = std::make_shared<Connection>(_ioc);
    _acceptor.async_accept(con_ptr->GetSocket(),[this,con_ptr](error_code err)
    {
        try
        {
            /* code */
            if(!err)
            {
                con_ptr->AsyncAccept();
            }
            else
            {
                std::cout<<"acceptor async_accept failded,err is "<<err.what()<<std::endl;
            }
            StartAccept();
        }
        catch(const std::exception& e)
        {
            std::cerr<<"WebSocketServer async accept excetion is" << e.what() << '\n';
        }
        
    });
}