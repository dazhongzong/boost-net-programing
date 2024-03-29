#pragma once
#include "Const.h"
#include "Singleton.h"
#include <queue>
#include <thread>
#include <map>
#include <memory>
#include <functional>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <condition_variable>
class CSession;
class LogicNode;


using FunCallBack = std::function<void(std::shared_ptr<CSession>,const short&,const std::string&)>;

class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;
public:
    ~LogicSystem();
    void PostMsgToQue(std::shared_ptr<LogicNode> msg);
private:
    LogicSystem();
    void RegisterCallbacks();//注册回调函数
    void HelloWordCallBack(std::shared_ptr<CSession> session,const short& msg_id,const std::string& msg_data);
    void DealMsg();


    std::queue<std::shared_ptr<LogicNode>> _msg_que;
    std::mutex _mutex;          // 网络层和逻辑层都会访问 ，生产者消费者
    std::condition_variable _consume;

    std::thread _worket_thread; //工作线程  从队列中取出数据
    bool _b_stop;               //停止信号
    std::map<short,FunCallBack> _fun_callback;      
};