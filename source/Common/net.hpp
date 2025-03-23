#pragma once

// 定义网络库

// muduo网络库
#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/TcpClient.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/net/EventLoopThread.h>

#include <iostream>
#include "fields.hpp"
#include "message.hpp"
#include "detail.hpp"

#include <thread>
#include <mutex>
#include <unordered_map>

namespace rpcframe
{
    // buffer block 设置缓冲区
    // 本质是对muduoBuffer的再封装
    class MuduoBuffer : public BaseBuffer
    {
    public:
        MuduoBuffer(muduo::net::Buffer *buffer) : _buffer(buffer)
        {
        }

        using Ptr = std::shared_ptr<MuduoBuffer>;
        // using constPtr = std::shared_ptr<const BaseBuffer>;

        virtual size_t readAbleSize() override
        {
            return _buffer->readableBytes();
        }
        // muduo库进行了网络字节序的装换
        virtual int32_t peekInt32() override
        {
            return _buffer->peekInt32();
        }

        virtual void retriveInt32() override
        {
            _buffer->retrieveInt32();
        }

        virtual int32_t readInt32() override
        {
            // peek + retrieve
            return _buffer->readInt32();
        }

        virtual std::string retriveAsString(size_t len) override
        {
            return _buffer->retrieveAsString(len);
        }

    private:
        // 指向的是muduo connection中的缓冲区
        muduo::net::Buffer *_buffer;
    };
    // buffer构建工厂

    class BufferFactory
    {
    public:
        template <class... Args>
        static MuduoBuffer::Ptr create(Args &&...args)
        {
            return std::make_shared<MuduoBuffer>(std::forward<Args>(args)...);
        }
    };

    // BaseProtocal
    // 实现应用层协议
    class LVProtocal : public BaseProtocal
    {
    public:
        using Ptr = std::shared_ptr<LVProtocal>;
        // 是否能够处理
        virtual bool canProcessed(const BaseBuffer::Ptr &buffer) override
        {
            // \--len--\
            // 1. 尝试取出4字节数据
            if(buffer->readAbleSize() < 4) return false; 
            int32_t total_len = buffer->peekInt32();
            if (total_len + lenSize > buffer->readAbleSize())
            {
                // 不足一条完整的数据
                return false;
            }

            // \--len--\--mtype--\--idlen--\--id--\--body--\

            return true;
        }

        // 这里有潜在的悬空的问题 --debug
        virtual bool onMessage(const BaseBuffer::Ptr &buf, BaseMessage::Ptr &msg) override
        {
            if (canProcessed(buf) == false)
                return false;
            // 调用onMessage,默认缓冲区内容足够处理
            std::cout << "size : " << buf->readAbleSize() << std::endl;
            int32_t total_len = buf->readInt32();
            Mtype m_type = (Mtype)buf->readInt32();
            int32_t id_len = buf->readInt32();
            //DLOG("totalLen:%d idLen:%d bodyLen:%d",total_len,id_len,total_len - id_len - idlenSize - mtypeSize);
            std::string id = buf->retriveAsString(id_len);
            
            int32_t body_len = total_len - id_len - idlenSize - mtypeSize;
            std::string body = buf->retriveAsString(body_len);
            // 根据消息类型构建对象
            msg = MessageFactory::create(m_type);
            if (msg.get() == nullptr)
            {
                ELOG("消息类型错误,解析构造消息对象失败");
                return false;
            }

            // 消息构建成功
            bool ret = msg->unserialize(body);
            if (ret == false)
            {
                ELOG("消息正文反序列化失败");
                return false;
            }
            // 设置id和mType
            msg->setId(id);
            msg->setMtype(m_type);
            return true;
        }
        virtual std::string serialize(const BaseMessage::Ptr &msg)
        {
            // \--len--\--mtype--\--idlen--\--id--\--body--\

            // 注意成转换成网络字节序
            std::string body = msg->serialize();
            int32_t body_len = body.size();
            std::string id = msg->rid();
            int32_t id_len = id.size();
            int32_t m_type = htonl((int32_t)msg->mType());
            int32_t n_id_len = ::htonl(id_len);
            int total_len = htonl(mtypeSize + idlenSize + id_len + body_len);
            std::string result;
            //DLOG("totalLen:%d idLen:%d bodyLen:%d",mtypeSize + idlenSize + id_len + body_len,id.size(),body.size());
            // 可能出现大小端的问题
            
            result.append((char *)&total_len, lenSize);
            result.append((char *)&m_type, mtypeSize);
            result.append((char *)&n_id_len, idlenSize);
            result.append(id);
            result.append(body);
            DLOG("%d",(int)result.size());
            return result;
        }

    private:
        const size_t lenSize = 4;

        const size_t mtypeSize = 4;

        const size_t idlenSize = 4;
    };
    class ProtocalFactory
    {
    public:
        template <class... Args>
        static LVProtocal::Ptr create(Args &&...args)
        {
            return std::make_shared<LVProtocal>(std::forward<Args>(args)...);
        }
    };

    class MuduoConnection : public BaseConnection
    {
    public:
        using Ptr = std::shared_ptr<MuduoConnection>;

        MuduoConnection(const muduo::net::TcpConnectionPtr &con, const BaseProtocal::Ptr &pro) : _conn(con), _protocal(pro)
        {
        }

        virtual void send(const BaseMessage::Ptr &msg) override
        {
            std::string send_msg = _protocal->serialize(msg);
            
            _conn->send(send_msg);
        }

        virtual void shutDown() override
        {
            _conn->shutdown();
        }

        virtual bool connected() const override
        {
            return _conn->connected();
        }

    private:
        // 基于protocal进行消息的派发
        BaseProtocal::Ptr _protocal;
        //
        muduo::net::TcpConnectionPtr _conn;
    };
    // connection构建工厂
    class ConnectionFactory
    {
    public:
        template <class... Args>
        static BaseConnection::Ptr create(Args &&...args)
        {
            return std::make_shared<MuduoConnection>(std::forward<Args>(args)...);
        }
    };

    // using ConnectionCallBack = std::function<void(const BaseConnection::Ptr&)>;
    // using CloseCallBack = std::function<void(const BaseConnection::Ptr&)>;
    // using MessageCallBack = std::function<void(const BaseConnection::Ptr&,BaseBuffer::Ptr&)>

    class MuduoServer : public BaseServer
    {
    public:
        using Ptr = std::shared_ptr<MuduoServer>;

        MuduoServer(int port) : _server(&_baseloop, muduo::net::InetAddress("0.0.0.0", port), "rpc Server", muduo::net::TcpServer::kReusePort),
                                _protocal(ProtocalFactory::create())
        {
        }
        // 设置回调函数,基类中定义
        // void setConnectionCallBack(const ConnectionCallBack& cnc_call_back) { _cnc_call_back = cnc_call_back; }

        // void setCloseCallBack(const CloseCallBack& cb) { _cls_call_back = cb; }

        // void setMessageCallBack(const MessageCallBack& cb) { _msg_call_back = cb; }

        // 启动服务器

        virtual void start()
        {
            // 设置回调函数
            _server.setConnectionCallback(std::bind(&MuduoServer::onConnection, this, std::placeholders::_1));
            _server.setMessageCallback(std::bind(&MuduoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _server.start();  // 开始监听
            DLOG("开始监听");
            _baseloop.loop(); // 开始事件循环
            
        }

    private:
        void onConnection(const muduo::net::TcpConnectionPtr &conn)
        {
            if (conn->connected())
            {
                // 连接建立
                ELOG("连接建立");
                auto moduo_conn = ConnectionFactory::create(conn, _protocal);
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    _conns.insert(std::make_pair(conn, moduo_conn));
                }
                if (_cnc_call_back)
                    _cnc_call_back(moduo_conn); // 用户设置了回调，
            }
            else
            {
                ELOG("连接断开");
                BaseConnection::Ptr muduo_conn;
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    auto it = _conns.find(conn);
                    if (it == _conns.end())
                    {
                        // 没找到
                        return;
                    }
                    muduo_conn = it->second;
                    _conns.erase(conn);
                    if (_cls_call_back)
                        _cls_call_back(muduo_conn); // 连接断开
                }
            }
        }

        void onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buffer, muduo::Timestamp)
        {
            DLOG("有新数据到来,开始处理");
            auto base_buf = BufferFactory::create(buffer);

            while (true)
            {
                if (_protocal->canProcessed(base_buf) == false)
                {
                    if (base_buf->readAbleSize() > maxDataSize)
                    {
                        con->shutdown();
                        ELOG("缓冲区中数据过大");
                        return;
                    }
                    // 数据不足
                    DLOG("数据量不足");
                    return;
                }

                BaseMessage::Ptr msg;
                bool ret = _protocal->onMessage(base_buf, msg);
                if (ret == false)
                {
                    ELOG("缓冲区中数据错误");
                    con->shutdown();
                    return;
                }
                DLOG("缓冲区中数据可处理");
                BaseConnection::Ptr base_con;
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    auto it = _conns.find(con);
                    if (it == _conns.end())
                    {
                        con->shutdown();
                        return;
                    }
                    base_con = it->second;
                }
                DLOG("调用回调函数进行消息处理");
                if (_msg_call_back)
                    _msg_call_back(base_con, msg);
            }
        }

    private:
        const size_t maxDataSize = 1 << 16;

        muduo::net::EventLoop _baseloop; // 事件循环监控
        muduo::net::TcpServer _server;   // 服务

        std::mutex _mtx;
        // 建立tcpconnection 和 baseConnection的映射
        std::unordered_map<muduo::net::TcpConnectionPtr, BaseConnection::Ptr> _conns;

        BaseProtocal::Ptr _protocal;
    };

    class ServerFactory
    {
    public:
        template <class... Args>
        static BaseServer::Ptr create(Args &&...args)
        {
            return std::make_shared<MuduoServer>(std::forward<Args>(args)...);
        }
    };

    class MuduoClient : public BaseClient
    {
    public:
        using Ptr = std::shared_ptr<MuduoClient>;

        MuduoClient(const std::string &sip, int port) : _baseloop(_thread.startLoop()),
                                                        _countdownlatch(1),
                                                        _client(_baseloop, muduo::net::InetAddress(sip, port), "Muduo client"),
                                                        _protocal(ProtocalFactory::create())
        {
        }
        // 启动服务器
        virtual void connect() override
        {
            _client.setConnectionCallback(std::bind(&MuduoClient::onConnection, this, std::placeholders::_1));
            _client.setMessageCallback(std::bind(&MuduoClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            // 连接服务器
            _client.connect();
            ELOG("开始等待");
            _countdownlatch.wait(); // 等待connect
            ELOG("等待连接成功");
        }
        virtual void shutdown() override
        {
            return _client.disconnect();
        }
        virtual bool send(const BaseMessage::Ptr & msg) override
        {
            if(connected() == false) return false;
            _conn->send(msg);
            return true;
        }
        // 获取连接
        virtual BaseConnection::Ptr connection() {
            return _conn;
        }
        virtual bool connected() const {
            return _conn && _conn->connected();
        }
    private:
        void onConnection(const muduo::net::TcpConnectionPtr &conn)
        {
            if (conn->connected())
            {
                ELOG("连接建立成功");
                _countdownlatch.countDown();
                _conn = ConnectionFactory::create(conn,_protocal);
            }
            else
            {
                std::cout << "连接断开" << std::endl;
                _conn.reset();
            }
        }

        void onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buffer, muduo::Timestamp)
        {
            auto base_buf = BufferFactory::create(buffer);

            while (true)
            {
                if (_protocal->canProcessed(base_buf) == false)
                {
                    if (base_buf->readAbleSize() > maxDataSize)
                    {
                        con->shutdown();
                        ELOG("缓冲区中数据过大");
                        return;
                    }
                    // 数据不足
                    return;
                }
                BaseMessage::Ptr msg;
                bool ret = _protocal->onMessage(base_buf, msg);
                if (ret == false)
                {
                    ELOG("缓冲区中数据错误");
                    con->shutdown();
                    return;
                }
                if(_msg_call_back) _msg_call_back(_conn,msg);
            }
        }

    private:
        const int maxDataSize = 1 << 16;
        BaseConnection::Ptr _conn;
        muduo::net::EventLoopThread _thread;
        muduo::CountDownLatch _countdownlatch;
        muduo::net::EventLoop *_baseloop;
        muduo::net::TcpClient _client;
        // 协议
        BaseProtocal::Ptr _protocal;
        // 锁
        std::mutex _mtx;
    };

    // 创建client工厂
    class ClientFactory
    {
    public:
        // MuduoClient(const std::string &sip, int port)
        template <class ...Args>
        static std::shared_ptr<BaseClient> create(Args&&... args)
        {
            return std::make_shared<MuduoClient>(std::forward<Args>(args)...);
        }
    };
}