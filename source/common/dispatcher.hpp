#pragma once
#include "net.hpp"
#include "message.hpp"

namespace bitrpc {
    class Callback {
        public:
            using ptr = std::shared_ptr<Callback>;
            virtual void onMessage(const BaseConnection::ptr &conn, BaseMessage::ptr &msg) = 0;
    };

    template<typename T>
    class CallbackT : public Callback{
        public:
            using ptr = std::shared_ptr<CallbackT<T>>;
            using MessageCallback = std::function<void(const BaseConnection::ptr &conn, std::shared_ptr<T> &msg)>;
            CallbackT(const MessageCallback &handler):_handler(handler) { }
            void onMessage(const BaseConnection::ptr &conn, BaseMessage::ptr &msg) override {
                auto type_msg = std::dynamic_pointer_cast<T>(msg);
                _handler(conn, type_msg);
            }
        private:
            MessageCallback _handler;
    };
    class Dispatcher {
        public:
            using ptr = std::shared_ptr<Dispatcher>;
            template<typename T>
            void registerHandler(MType mtype, const typename CallbackT<T>::MessageCallback &handler) {
                std::unique_lock<std::mutex> lock(_mutex);
                auto cb = std::make_shared<CallbackT<T>>(handler);
                _handlers.insert(std::make_pair(mtype, cb));
            }
            void onMessage(const BaseConnection::ptr &conn, BaseMessage::ptr &msg) {
                //找到消息类型对应的业务处理函数，进行调用即可
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _handlers.find(msg->mtype());
                if (it != _handlers.end()) {
                    return it->second->onMessage(conn, msg);
                }
                //没有找到指定类型的处理回调--因为客户端和服务端都是我们自己设计的，因此不可能出现这种情况
                ELOG("收到未知类型的消息: %d！", msg->mtype());
                conn->shutdown();
            }
        private:
            std::mutex _mutex;
            std::unordered_map<MType, Callback::ptr> _handlers;
    };
}