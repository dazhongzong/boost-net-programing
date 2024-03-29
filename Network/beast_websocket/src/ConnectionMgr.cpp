#include "../inc/ConnectionMgr.h"

ConnectionMgr& ConnectionMgr::GetInstance()
{
    static ConnectionMgr instance;
    return instance;
}

ConnectionMgr::ConnectionMgr()
{

}

void ConnectionMgr::AddConnection(std::shared_ptr<Connection> conptr)
{
    _map_conns[conptr->GetUuid()] = conptr;
}

void ConnectionMgr::RemoveConnection(std::string uuid)
{
    if(_map_conns.find(uuid)!= _map_conns.end())
        _map_conns.erase(uuid);
}