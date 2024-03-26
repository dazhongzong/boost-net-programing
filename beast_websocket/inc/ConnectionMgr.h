#pragma once
#include "Connection.h"
#include <unordered_map>

class ConnectionMgr
{
public:
    static ConnectionMgr& GetInstance();
    void AddConnection(std::shared_ptr<Connection> conptr);
    void RemoveConnection(std::string uuid);
private:
    ConnectionMgr();
    ConnectionMgr(const ConnectionMgr&) = delete;
    ConnectionMgr& operator=(const ConnectionMgr&) = delete;
    std::unordered_map<std::string,std::shared_ptr<Connection>> _map_conns;
};