// 定义我们的抽象层
#pragma once
#include <memory>
#include <functional>
#include "fields.hpp"

namespace rpcframe
{
    // 对req的抽象
    class BaseMessage
    {
    public:
        using Ptr = std::shared_ptr<BaseMessage>;    

        virtual ~BaseMessage() {}

        virtual void setId(const std::string& id) { _rid = id; }

        virtual std::string rid() const { return _rid; }
        
        virtual void setMtype(Mtype mtype) { _mtype = mtype; }

        virtual Mtype mType() { return _mtype; }

        virtual std::string serialize() = 0;

        virtual bool unserialize(const std::string& msg) = 0;

        virtual bool check() const = 0;
    private:
        Mtype _mtype;
        std::string _rid;
    };
}

namespace rpcframe
{
    // 对缓冲区的抽象

    class BaseBuffer
    {
    public:
        using Ptr = std::shared_ptr<BaseBuffer>;
        // using constPtr = std::shared_ptr<const BaseBuffer>;

        virtual size_t readAbleSize() = 0;

        virtual int32_t peekInt32() = 0;

        virtual void retriveInt32() = 0;

        virtual int32_t readInt32() = 0;

        virtual std::string retriveAsString(size_t len) = 0;


    private:
        
    };
    
}

namespace rpcframe
{
    class BaseProtocal
    {
    public:
        using Ptr = std::shared_ptr<BaseProtocal>;
        // 是否能够处理
        virtual bool canProcessed(const BaseBuffer::Ptr&) = 0;

        // 这里有潜在的悬空的问题 --debug
        virtual bool onMessage(const BaseBuffer::Ptr&,BaseBuffer::Ptr&) = 0;

        virtual std::string serialize(const BaseMessage::Ptr&) = 0;
    private:
    };
}

namespace rpcframe
{
    class BaseConnection
    {
    public:
        using Ptr = std::shared_ptr<BaseConnection>;

        virtual void send(const BaseMessage::Ptr&) = 0;

        virtual void shutDown() = 0;
        
        virtual bool connected() const = 0;
    private:
        // 针对消息进行序列化
        BaseProtocal::Ptr _protocal;
    };


    using ConnectionCallBack = std::function<void(const BaseConnection::Ptr&)>;
    using CloseCallBack = std::function<void(const BaseConnection::Ptr&)>;
    using MessageCallBack = std::function<void(const BaseConnection::Ptr&,BaseBuffer::Ptr&)>;

    
    class BaseServer
    {
    public:
        using Ptr = std::shared_ptr<BaseServer>;

        // 设置回调函数
        void setConnectionCallBack(const ConnectionCallBack& cnc_call_back) { _cnc_call_back = cnc_call_back; }

        void setCloseCallBack(const CloseCallBack& cb) { _cls_call_back = cb; }

        void setMessageCallBack(const MessageCallBack& cb) { _msg_call_back = cb; }

        // 启动服务器
        virtual void start() = 0;
    private:
        ConnectionCallBack _cnc_call_back;
        CloseCallBack _cls_call_back;
        MessageCallBack _msg_call_back;
    };
}

// 客户端的抽象

namespace rpcframe
{

    class BaseClient
    {
    public:
        using Ptr = std::shared_ptr<BaseClient>;

        // 设置回调函数
        void setConnectionCallBack(const ConnectionCallBack& cnc_call_back) { _cnc_call_back = cnc_call_back; }

        void setCloseCallBack(const CloseCallBack& cb) { _cls_call_back = cb; }

        void setMessageCallBack(const MessageCallBack& cb) { _msg_call_back = cb; }

        // 启动服务器
        virtual void connect() = 0;
        virtual void shutdown() = 0;
        virtual void send(const BaseMessage::Ptr&) = 0;
        // 获取连接
        virtual BaseConnection::Ptr connection() = 0;

        virtual bool connected() const = 0;
    private:
        ConnectionCallBack _cnc_call_back;
        CloseCallBack _cls_call_back;
        MessageCallBack _msg_call_back;
    };
}