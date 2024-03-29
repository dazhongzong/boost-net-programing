#pragma once
#include <string>
#include <iostream>
#include <boost/asio.hpp>

class MsgNode
{
public:
    MsgNode(short total_len);

    virtual ~MsgNode();

    void Clear();

    short _cur_len;
    short _total_len;
    char* _data;
};

class RecvNode : public MsgNode
{
public:
    RecvNode(short total_len,short msg_id);
    ~RecvNode()
    {

    }
private:
    short _msg_id;
};

class SendNode: public MsgNode
{
public:
    SendNode(const char*msg,short total_len,short msg_id);
    ~SendNode()
    {

    }
private:
    short _msg_id;
};