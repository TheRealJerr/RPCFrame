#pragma once
#include "../common/net.hpp"
#include "../common/message.hpp"
#include <unordered_set>

namespace bitrpc {
    namespace server {
        class TopicManager {
            public:
                using ptr = std::shared_ptr<TopicManager>;
                TopicManager() {}
                void onTopicRequest(const BaseConnection::ptr &conn, const TopicRequest::ptr &msg) {
                    TopicOptype topic_optype = msg->optype();
                    bool ret = true;
                    switch(topic_optype){
                        //主题的创建
                        case TopicOptype::TOPIC_CREATE: topicCreate(conn, msg); break;
                        //主题的删除
                        case TopicOptype::TOPIC_REMOVE: topicRemove(conn, msg); break;
                        //主题的订阅
                        case TopicOptype::TOPIC_SUBSCRIBE: ret = topicSubscribe(conn, msg); break;
                        //主题的取消订阅
                        case TopicOptype::TOPIC_CANCEL: topicCancel(conn, msg); break;
                        //主题消息的发布
                        case TopicOptype::TOPIC_PUBLISH: ret = topicPublish(conn, msg); break;
                        default:  return errorResponse(conn, msg, RCode::RCODE_INVALID_OPTYPE);
                    }
                    if (!ret) return errorResponse(conn, msg, RCode::RCODE_NOT_FOUND_TOPIC);
                    return topicResponse(conn, msg);
                }
                //一个订阅者在连接断开时的处理---删除其关联的数据
                void onShutdown(const BaseConnection::ptr &conn) {
                    //消息发布者断开连接，不需要任何操作；  消息订阅者断开连接需要删除管理数据
                    //1. 判断断开连接的是否是订阅者，不是的话则直接返回
                    std::vector<Topic::ptr> topics;
                    Subscriber::ptr subscriber;
                    {
                        std::unique_lock<std::mutex> lock(_mutex);
                        auto it = _subscribers.find(conn);
                        if (it == _subscribers.end()) {
                            return;//断开的连接，不是一个订阅者的连接
                        }
                        subscriber = it->second;
                        //2. 获取到订阅者退出，受影响的主题对象
                        for (auto &topic_name : subscriber->topics) {
                            auto topic_it = _topics.find(topic_name);
                            if (topic_it == _topics.end()) continue;
                            topics.push_back(topic_it->second);
                        }
                        //4. 从订阅者映射信息中，删除订阅者
                        _subscribers.erase(it);
                    }
                    //3. 从受影响的主题对象中，移除订阅者
                    for (auto &topic : topics) {
                        topic->removeSubscriber(subscriber);
                    }
                }
            private:
                void errorResponse(const BaseConnection::ptr &conn, const TopicRequest::ptr &msg, RCode rcode) {
                    auto msg_rsp = MessageFactory::create<TopicResponse>();
                    msg_rsp->setId(msg->rid());
                    msg_rsp->setMType(MType::RSP_TOPIC);
                    msg_rsp->setRCode(rcode);
                    conn->send(msg_rsp);
                }
                void topicResponse(const BaseConnection::ptr &conn, const TopicRequest::ptr &msg) {
                    auto msg_rsp = MessageFactory::create<TopicResponse>();
                    msg_rsp->setId(msg->rid());
                    msg_rsp->setMType(MType::RSP_TOPIC);
                    msg_rsp->setRCode(RCode::RCODE_OK);
                    conn->send(msg_rsp);
                }
                void topicCreate(const BaseConnection::ptr &conn, const TopicRequest::ptr &msg) {
                    std::unique_lock<std::mutex> lock(_mutex);
                    //构造一个主题对象，添加映射关系的管理
                    std::string topic_name = msg->topicKey();
                    auto topic = std::make_shared<Topic>(topic_name);
                    _topics.insert(std::make_pair(topic_name, topic));
                }
                void topicRemove(const BaseConnection::ptr &conn, const TopicRequest::ptr &msg) {
                    // 1. 查看当前主题，有哪些订阅者，然后从订阅者中将主题信息删除掉
                    // 2. 删除主题的数据 -- 主题名称与主题对象的映射关系
                    std::string topic_name = msg->topicKey();
                    std::unordered_set<Subscriber::ptr> subscribers;
                    {
                        std::unique_lock<std::mutex> lock(_mutex);
                        //在删除主题之前，先找出会受到影响的订阅者
                        auto it = _topics.find(topic_name);
                        if (it == _topics.end()) {
                            return;
                        }
                        subscribers = it->second->subscribers;
                        _topics.erase(it);//删除当前的主题映射关系，
                    }
                    for (auto &subscriber : subscribers) {
                        subscriber->removeTopic(topic_name);
                    }
                }
                bool topicSubscribe(const BaseConnection::ptr &conn, const TopicRequest::ptr &msg) {
                    //1. 先找出主题对象，以及订阅者对象
                    //   如果没有找到主题--就要报错；  但是如果没有找到订阅者对象，那就要构造一个订阅者
                    Topic::ptr topic;
                    Subscriber::ptr subscriber;
                    {
                        std::unique_lock<std::mutex> lock(_mutex);
                        auto topic_it = _topics.find(msg->topicKey());
                        if (topic_it == _topics.end()) {
                            return false;
                        }
                        topic = topic_it->second;
                        auto sub_it = _subscribers.find(conn);
                        if (sub_it != _subscribers.end()) {
                            subscriber = sub_it->second;
                        }else {
                            subscriber = std::make_shared<Subscriber>(conn);
                            _subscribers.insert(std::make_pair(conn, subscriber));
                        }
                    }
                    //2. 在主题对象中，新增一个订阅者对象关联的连接；  在订阅者对象中新增一个订阅的主题
                    topic->appendSubscriber(subscriber);
                    subscriber->appendTopic(msg->topicKey());
                    return true;
                }
                void topicCancel(const BaseConnection::ptr &conn, const TopicRequest::ptr &msg) {
                    //1. 先找出主题对象，和订阅者对象
                    Topic::ptr topic;
                    Subscriber::ptr subscriber;
                    {
                        std::unique_lock<std::mutex> lock(_mutex);
                        auto topic_it = _topics.find(msg->topicKey());
                        if (topic_it != _topics.end()) {
                            topic =  topic_it->second;
                        }
                        auto sub_it = _subscribers.find(conn);
                        if (sub_it != _subscribers.end()) {
                            subscriber = sub_it->second;
                        }
                    }
                    //2. 从主题对象中删除当前的订阅者连接；   从订阅者信息中删除所订阅的主题名称
                    if (subscriber) subscriber->removeTopic(msg->topicKey());
                    if (topic && subscriber) topic->removeSubscriber(subscriber);
                }
                bool topicPublish(const BaseConnection::ptr &conn, const TopicRequest::ptr &msg) {
                    Topic::ptr topic;
                    {
                        std::unique_lock<std::mutex> lock(_mutex);
                        auto topic_it = _topics.find(msg->topicKey());
                        if (topic_it == _topics.end()) {
                            return false;
                        }
                        topic = topic_it->second;
                    }
                    topic->pushMessage(msg);
                    return true;
                }
            private:
                struct Subscriber {
                    using ptr = std::shared_ptr<Subscriber>;
                    std::mutex _mutex;
                    BaseConnection::ptr conn;
                    std::unordered_set<std::string> topics;//订阅者所订阅的主题名称

                    Subscriber(const BaseConnection::ptr &c): conn(c) { }
                    //订阅主题的时候调用
                    void appendTopic(const std::string &topic_name) {
                        std::unique_lock<std::mutex> lock(_mutex);
                        topics.insert(topic_name);
                    }
                    //主题被删除 或者 取消订阅的时候，调用
                    void removeTopic(const std::string &topic_name) {
                        std::unique_lock<std::mutex> lock(_mutex);
                        topics.erase(topic_name);
                    }
                };
                struct Topic {
                    using ptr = std::shared_ptr<Topic>;
                    std::mutex _mutex;
                    std::string topic_name;
                    std::unordered_set<Subscriber::ptr> subscribers; //当前主题的订阅者

                    Topic(const std::string &name) : topic_name(name){}
                    //新增订阅的时候调用
                    void appendSubscriber(const Subscriber::ptr &subscriber) {
                        std::unique_lock<std::mutex> lock(_mutex);
                        subscribers.insert(subscriber);
                    }
                    //取消订阅 或者 订阅者连接断开 的时候调用
                    void removeSubscriber(const Subscriber::ptr &subscriber) {
                        std::unique_lock<std::mutex> lock(_mutex);
                        subscribers.erase(subscriber);
                    }
                    //收到消息发布请求的时候调用
                    void pushMessage(const BaseMessage::ptr &msg) {
                        std::unique_lock<std::mutex> lock(_mutex);
                        for (auto &subscriber : subscribers) {
                            subscriber->conn->send(msg);
                        }
                    }
                };
            private:
                std::mutex _mutex;
                std::unordered_map<std::string, Topic::ptr> _topics;
                std::unordered_map<BaseConnection::ptr, Subscriber::ptr> _subscribers;
        };
    }
}