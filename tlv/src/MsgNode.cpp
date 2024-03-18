#include "../inc/MsgNode.h"
#include <iostream>
#include "../inc/Const.h"
MsgNode::MsgNode(short total_len):
    _cur_len(0),
    _total_len(total_len),
    _data(new char[_total_len + 1]())
{
    _data[total_len] = '\0';
}

MsgNode::~MsgNode()
{
    if(_data)
    {
        std::cout<<"destruct MsgNode" <<std::endl;
        delete[] _data;
        _data = nullptr;
    }
}

void MsgNode::Clear()
{
    memset(_data,0,_total_len);
    _cur_len = 0;
}

RecvNode::RecvNode(short total_len,short msg_id):MsgNode(total_len),_msg_id(msg_id)
{

}

SendNode::SendNode(const char* msg,short total_len,short msg_id):MsgNode(total_len+HEAD_TOTAL_LEN),_msg_id(msg_id)
{
    //tlv 格式 type length value
    short msg_id_host = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
    memcpy(_data,&msg_id_host,HEAD_ID_LEN);
    
    short total_len_host = boost::asio::detail::socket_ops::host_to_network_short(total_len);
    memcpy(_data+HEAD_ID_LEN,&total_len_host,HEAD_DATA_LEN);

    memcpy(_data+HEAD_TOTAL_LEN,&msg,total_len);
}