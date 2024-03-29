#include "../inc/MsgNode.h"

RecvNode::RecvNode(short max_len,short msg_id):MsgNode(max_len),_msg_id(msg_id)
{

}

SendNode::SendNode(const char* msg,short max_len,short msg_id):MsgNode(max_len+HEAD_TOTAL),_msg_id(msg_id)
{
    //发送ID
    short msg_id_net = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
    memcpy(_data,&msg_id,HEAD_ID_LEN);
    //发动数据长度
    short head_data_len_net = boost::asio::detail::socket_ops::host_to_network_short(max_len);
    memcpy(_data + HEAD_ID_LEN,&head_data_len_net,HEAD_DATA_LEN);

    //发送数据
    memcpy(_data + HEAD_TOTAL,msg,max_len);
}