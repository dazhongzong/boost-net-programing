#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "../inc/demo.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using hello::HelloRequest;
using hello::HelloReply;

using hello::Greeter;

class GreeterServiceImpl final: public Greeter::Service
{
    ::grpc::Status SayHello(::grpc::ServerContext* context, const ::hello::HelloRequest* request, ::hello::HelloReply* response)
    {
        std::string prefix("llfc grpc server has received: ");
        response->set_message(prefix + request->message());
        return ::grpc::Status::OK;
    }

};

void RunServer()
{
    std::string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;
    ServerBuilder builder;
    builder.AddListeningPort(server_address,::grpc::InsecureServerCredentials());       //不做服务器校验
    builder.RegisterService(&service);

    std::unique_ptr<::grpc::Server> server(builder.BuildAndStart());
    std::cout<<"Server listening on "<<server_address <<std::endl;
    server->Wait();
}

int main()
{
    RunServer();

    return 0;
}