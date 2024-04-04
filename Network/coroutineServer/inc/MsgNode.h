#pragma once
#include <string>
#include "Const.h"
#include <iostream>
#include <boost/asio.hpp>


//消息节点
class MsgNode
{
public:
    MsgNode(short max_len):_cur_len(0),_total_len(max_len),_data(new char[_total_len+1](0)){}

    virtual ~MsgNode()
    {
        if(_data)
        {
            std::cout<<"destruct MsgNode"<<std::endl;
            delete[] _data;
            _data = nullptr;
        }
    }

    void Clear()
    {
        _cur_len = 0;
        memset(_data,0,_total_len+1);
    }

    short _cur_len;
    short _total_len;
    char* _data;
};

class RecvNode:public MsgNode
{
public:
    RecvNode(short max_len,short msg_id);
    short _msg_id;
};

class SendNode:public MsgNode
{
public:
    SendNode(const char* msg,short max_len,short msg_id);
    
    short _msg_id;
};