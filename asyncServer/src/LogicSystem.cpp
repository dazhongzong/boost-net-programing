#include "../inc/LogicSystem.h"
#include "../inc/CSession.h"
LogicSystem::LogicSystem():_b_stop(false)
{
    RegisterCallbacks();        //将消息id 与回调函数绑定
    _worket_thread = std::thread(&LogicSystem::DealMsg,this);
}

void LogicSystem::RegisterCallbacks()
{
   _fun_callback[MSG_HELLO_WORD] = std::bind(&LogicSystem::HelloWordCallBack,this,
        std::placeholders::_1,std::placeholders::_2,std::placeholders::_3); 
}

void LogicSystem::HelloWordCallBack(std::shared_ptr<CSession> session,const short& msg_id,const std::string& msg_data)
{
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data,root);
    std::cout<<"receive msg id is "<<root["id"].asInt() << " msg data is "<<root["data"].asString()<<std::endl;
    root["data"] = "server has receive msg, msg data is " + root["data"].asString();

    std::string response = root.toStyledString();
    session->Send(response,root["id"].asInt());
}

void LogicSystem::DealMsg()
{
    for(;;)
    {
        std::unique_lock<std::mutex> lock(_mutex);      //配合 条件变量

        //判断队列为空则用条件变量等待
        _consume.wait(lock,[&](){
            return !(_msg_que.empty()&&!_b_stop);
        });

        //判断如果为关闭状态，取出逻辑队列所有数据及时处理并退出循环
        if(_b_stop)
        {
            while(!_msg_que.empty())
            {
                auto msg_node = _msg_que.front();
                std::cout<<"recv msg id "<<msg_node->_recvnode->_msg_id <<std::endl;

                auto call_back_iter = _fun_callback.find(msg_node->_recvnode->_msg_id);
                if(call_back_iter == _fun_callback.end())
                {
                    _msg_que.pop();
                    continue;
                }
                call_back_iter->second(msg_node->_session,msg_node->_recvnode->_msg_id,
                    std::string(msg_node->_recvnode->_data,msg_node->_recvnode->_cur_len));
                
                _msg_que.pop();
            }
            break;
        }

        //队列不为空 并且不是关闭状态
        auto msg_node = _msg_que.front();
        
        std::cout<<"recv msg id "<<msg_node->_recvnode->_msg_id <<std::endl;

        auto call_back_iter = _fun_callback.find(msg_node->_recvnode->_msg_id);
        if(call_back_iter == _fun_callback.end())
        {
            _msg_que.pop();
            continue;
        }
        call_back_iter->second(msg_node->_session,msg_node->_recvnode->_msg_id,
            std::string(msg_node->_recvnode->_data,msg_node->_recvnode->_cur_len));
        
        _msg_que.pop();
    }
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg)
{
    //生产者
    std::unique_lock<std::mutex> lock(_mutex);
    _msg_que.push(msg);

    if(_msg_que.size() == 1)
    {
        _consume.notify_one();
    }
}

LogicSystem::~LogicSystem()
{
    _b_stop = true;
    _consume.notify_one();
    _worket_thread.join();
}