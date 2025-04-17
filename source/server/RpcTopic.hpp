#pragma once
#include "../Common/message.hpp"
#include "../Common/net.hpp"
#include "../Common/dispatcher.hpp"
#include "RpcRouter.hpp"
#include "RpcRigister.hpp"
#include "../client/RpcClient.hpp"
#include "../Common/message.hpp"
// 向外提供主题操控的接口
namespace rpcframe
{
    namespace server
    {
        template <class T>
        class PtrHash
        {
        public:
            size_t operator()(const std::shared_ptr<T>& ptr) const 
            {
                return _hash(ptr.get());
            }
        private:
            std::hash<T*> _hash;
        };
        class TopicManager
        {
        public:
            using Ptr = std::shared_ptr<TopicManager>;
            // 接收到了主题的消息
            // 这个接口解释给dispatcher模块设计的
            void onTopicMessage(const BaseConnection::Ptr& con,TopicRequest::Ptr& msg)
            {
                // 主题的创建
                // 主题的删除
                // 主题的订阅
                // 主题的取消订阅
                // 主题的消息发布

                auto op_type = msg->topicOpType();
                bool ret = true;
                switch (op_type)
                {
                    case TopicOptType::TOPIC_CREATE : createTopic(con,msg); break;  
                    case TopicOptType::TOPIC_REMOVE : removeTopic(con,msg); break;
                    case TopicOptType::TOPIC_SUBCRIBE : ret = subscribeTopic(con,msg); break;
                    case TopicOptType::TOPIC_CANCEL : ret = cancelRemoveTopic(con,msg); break;
                    case TopicOptType::TOPIC_PUBLISH : ret = publishTopic(con,msg); break;
                    default : ELOG("topicoptype error: 未知的主题操作类型"); return errorResponse(con,msg,RCode::RCODE_INVALID_OPTYPE);
                }
                // 组织回复请求
                // ret = false; 没有对应的主题 
                if(ret) return topicResponse(con,msg);

                else return errorResponse(con,msg,RCode::RCODE_NOT_FIND_TOPIC);

            }

            // 订阅者连接断开,删除内部维护的订阅者的信息
            void onShutDown(const BaseConnection::Ptr& con)
            {
                // 消息订阅者,订阅者断开连接后
                // 获取到订阅者关联的所有的主题,从对应的主题对象中删除订阅者
                std::vector<Topic::Ptr> topics;
                Subscribe::Ptr subscribe;
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    if(_subscribes.count(con) == 0) return; // 不是订阅者的连接
                    subscribe = _subscribes[con];
                    for(auto& topic_name : subscribe->getTopics())
                        if(_topics.count(topic_name) > 0) topics.push_back(_topics[topic_name]);
                    // 从订阅者中删除
                    _subscribes.erase(con); // 将订阅者对象删除
                }
                for(auto& topic : topics)
                    topic->removeSubscribe(subscribe);

            }
            
            // 主题的创建
            void createTopic(const BaseConnection::Ptr& con,TopicRequest::Ptr& msg)
            {
                std::unique_lock<std::mutex> lock(_mtx);
                // 构造一个主题对象
                std::string topic_name = msg->topicKey();
                auto topic = std::make_shared<Topic>(topic_name);
                _topics.insert({ topic_name,topic }); // make_pair  
            }
            // 主题的删除
            void removeTopic(const BaseConnection::Ptr& con,TopicRequest::Ptr& msg)
            {
                Topic::Ptr topic;
                std::unordered_set<Subscribe::Ptr,PtrHash<Subscribe>> subscribes;
                // 查看主题的订阅者,从订阅者中将主题信息删除 
                auto topic_name = msg->topicKey(); 
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    if(_topics.count(topic_name) == 0) return; // 没有对应的主题
                    topic = _topics[topic_name];
                    
                    subscribes = topic->_subscribers;
                }
                for(auto& scr : subscribes)
                    scr->removeTopic(topic_name);
            }
        private:
            void topicResponse(const BaseConnection::Ptr &con, const TopicRequest::Ptr &msg)
            {
                auto msg_rsp = MessageFactory::create<TopicResponse>();
                msg_rsp->setId(msg->rid());
                msg_rsp->setMtype(Mtype::RSP_TOP);
                msg_rsp->setRCode(RCode::RCODE_OK);
                con->send(msg_rsp);
            }

            
            void errorResponse(const BaseConnection::Ptr& con,const TopicRequest::Ptr& msg,RCode rcode)
            {
                auto msg_rsp = MessageFactory::create<TopicResponse>();
                msg_rsp->setId(msg->rid());
                msg_rsp->setMtype(Mtype::RSP_TOP);
                msg_rsp->setRCode(rcode);
                con->send(msg_rsp);
            }
            // 主题的订阅
            bool subscribeTopic(const BaseConnection::Ptr& con,TopicRequest::Ptr& msg)
            {
                // 主题对象中新增连接
                // 订阅者中新增一个主题
                Topic::Ptr topic;
                Subscribe::Ptr subscribe;
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    if(_topics.count(msg->topicKey()) == 0) return false;
                    topic = _topics[msg->topicKey()];
                    if(_subscribes.count(con) == 0) return false;
                    subscribe = _subscribes[con];
                }
                // 找到了主题对象和订阅者对象
                topic->appendNewSubscribe(subscribe);
                subscribe->appendNewTopic(msg->topicKey());
                return true;
            }
            // 主题的取消订阅
            bool cancelRemoveTopic(const BaseConnection::Ptr& con,TopicRequest::Ptr& msg)
            {
                // 找出主题对象,和订阅者对象
                // 主题不存在报错,订阅者不存在则返回(不需要报错)
                Topic::Ptr topic;
                Subscribe::Ptr subscribe;
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    if(_topics.count(msg->topicKey()) == 0) return false;
                    topic = _topics[msg->topicKey()];
                    if(_subscribes.count(con) == 0) return false;
                    subscribe = _subscribes[con];
                }
                // 找到了主题对象和订阅者对象
                topic->removeSubscribe(subscribe);
                subscribe->removeTopic(msg->topicKey());
                return true;
            }
            // 主题的消息发布
            bool publishTopic(const BaseConnection::Ptr& con,TopicRequest::Ptr& msg)
            {
                Topic::Ptr topic;
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    if(_topics.count(msg->topicKey()) == 0) return false; // 没找到
                    topic = _topics[msg->topicKey()]; 
                }                
                topic->pushMessage(msg);
                return true;
            }
        private:
            // 订阅者
            
            class Subscribe
            {
            private:
                BaseConnection::Ptr _con;
                // 订阅者订阅的主题
                std::unordered_set<std::string> _topics;
                std::mutex _mtx;
                //
            public:
                const std::unordered_set<std::string>& getTopics() const { return _topics; }

                using Ptr = std::shared_ptr<Subscribe>;
                Subscribe(const BaseConnection::Ptr& con) : _con(con)
                {}
                void appendNewTopic(const std::string& topic)
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    _topics.insert(topic);
                }
                void removeTopic(const std::string& topic)
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    _topics.insert(topic);
                }
                const BaseConnection::Ptr& getCon() const { return _con; }
            };
            // 主题
            class Topic
            {
            private:
                std::mutex _mtx;
            public:
                std::string _topic_name;// 主题的名称
                // 当前主题管理的订阅者
                std::unordered_set<Subscribe::Ptr,PtrHash<Subscribe>> _subscribers;
            public:
                Topic(const std::string& name): _topic_name(name)
                {}
                using Ptr = std::shared_ptr<Topic>;
                // 添加订阅者
                void appendNewSubscribe(const Subscribe::Ptr& con)
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    _subscribers.insert(con);
                }
                void removeSubscribe(const Subscribe::Ptr& con)
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    _subscribers.erase(con);
                }
                // 主题获得消息的时候,对所有的订阅者发送消息
                void pushMessage(const BaseMessage::Ptr& msg)
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    for(auto& subscribe : _subscribers)
                    {
                        subscribe->getCon()->send(msg);
                    }
                }
                
            };

        private:
            std::mutex _mtx;
            std::unordered_map<std::string,Topic::Ptr> _topics;
            std::unordered_map<BaseConnection::Ptr,Subscribe::Ptr,PtrHash<BaseConnection>> _subscribes;
        };
    }
}