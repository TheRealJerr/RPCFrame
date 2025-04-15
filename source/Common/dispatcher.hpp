#pragma once 
#include <memory>
#include <functional>
#include "detail.hpp"
#include "fields.hpp"
#include "abstract.hpp"
#include "message.hpp"
#include "net.hpp"
namespace rpcframe
{
    // 通过多态实现基类指向派生类
    class CallBack
    {
    public:
        using Ptr = std::shared_ptr<CallBack>;
        virtual void onMessage(const BaseConnection::Ptr&,BaseMessage::Ptr& msg) = 0;
    };


    // 通过模版参数，指定消息类型
    template <class T>
    class CallBackDerive : public CallBack
    {
    public:
        using Ptr = std::shared_ptr<CallBackDerive<T>>;
        using MessageCallBack = std::function<void(const BaseConnection::Ptr&,std::shared_ptr<T>&)>;
        CallBackDerive(const MessageCallBack& handler)
            :_handler(handler)
        {}

        virtual void onMessage(const BaseConnection::Ptr& con,BaseMessage::Ptr& msg) override
        {
            // 类型检查,指针类型装换
            auto type_msg = std::dynamic_pointer_cast<T>(msg);
            // 调用
            _handler(con,type_msg);
        }
    private:
        MessageCallBack _handler;
    };


    class DisPatcher
    {
    public:
        using Ptr = std::shared_ptr<DisPatcher>;
        
        // 注册新的功能
        template <class T>
        void rigisterHandler(Mtype type,const typename CallBackDerive<T>::MessageCallBack & handler)
        {
            {
                std::unique_lock<std::mutex> lock(_mtx);
                // 注册新的功能
                auto cb = std::make_shared<CallBackDerive<T>>(handler);
                _handlers.insert(std::make_pair(type,cb));
            }
        }
        
        void onMessage(const BaseConnection::Ptr& con,BaseMessage::Ptr& msg)
        {
            {
                std::unique_lock<std::mutex> lock(_mtx);
                auto it = _handlers.find(msg->mType());
                if(it != _handlers.end()){
                    return it->second->onMessage(con,msg);
                }
                // 没有找到

                DLOG("未知的消息:%d",(int)msg->mType());
                con->shutDown(); // 关闭连接
            }
        }
    private:
        std::mutex _mtx;
        std::unordered_map<Mtype,CallBack::Ptr> _handlers;
    };
}