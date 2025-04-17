#pragma once

#include "requestor.hpp"
#include <unordered_set>
namespace rpcframe
{
    namespace client
    {
        class TopicManager
        {
        public:
            using Ptr = std::shared_ptr<TopicManager>;

            using SubCallBack = std::function<void(const std::string&,const std::string&)>; // 作为订阅者
            TopicManager(const Requestor::Ptr& requestor) : _requestor(requestor)
            {}

            bool createTopic(const BaseConnection::Ptr& con,std::string& method)
            {
                // 构造请求
                return sendMessage(con,method,TopicOptType::TOPIC_CREATE);
            }

            bool removeTopic(const BaseConnection::Ptr& con,std::string& method)
            {
                return sendMessage(con,method,TopicOptType::TOPIC_REMOVE);
            }

            bool subscribeTopic(const BaseConnection::Ptr& con,std::string& method,const SubCallBack& cb)
            {
                appendSubscribe(method,cb);
                bool ret = sendMessage(con,method,TopicOptType::TOPIC_SUBCRIBE);
                if(ret == false)
                {
                    removeSubscribe(method);
                    return false;
                }
                return true;
            }

            bool cancelTopic(const BaseConnection::Ptr& con,std::string& method)
            {
                removeSubscribe(method);
                return sendMessage(con,method,TopicOptType::TOPIC_CANCEL);
            }

            bool publishTopic(const BaseConnection::Ptr& con,std::string& method,const std::string& msg)
            {
                return sendMessage(con,method,TopicOptType::TOPIC_PUBLISH,msg);
            }

            void onPublish(const BaseConnection::Ptr& con,TopicRequest::Ptr& msg)
            {
                // 针对发送的消息 进行 回调处理
                auto type = msg->topicOpType();
                if(type != TopicOptType::TOPIC_PUBLISH)
                {
                    ELOG("收到了错误类型的消息");
                    return;
                }
                std::string topic_method = msg->topicKey();
                std::string topic_msg = msg->topicMsg();
                const auto& cb = getCallBack(topic_method);
                if(!cb)
                {
                    ELOG("收到了publish消息,但是针对%s主题没有对应的回调处理函数",topic_method.c_str());
                    return;
                }
                return cb(topic_method,topic_msg);
            }

        private:
            void appendSubscribe(std::string& method,const SubCallBack& cb)
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _topic_callbacks.insert(std::make_pair(method,cb));
            }
            void removeSubscribe(std::string& method)
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _topic_callbacks.erase(method);
            }

            const SubCallBack& getCallBack(std::string& method)
            {
                std::unique_lock<std::mutex> lock(_mtx);
                if(_topic_callbacks.count(method) == 0) return SubCallBack();
                return _topic_callbacks[method];
            }

            bool sendMessage(const BaseConnection::Ptr& con,const std::string& method,TopicOptType opt,const std::string& msg = std::string())
            {
                auto msg_req = MessageFactory::create<TopicRequest>();
                msg_req->setId(UUIDTool::getUUID());
                msg_req->setMtype(Mtype::REQ_TOP);
                msg_req->setTopicOptType(opt);
                msg_req->setTopicKey(method);
                // 
                if(opt == TopicOptType::TOPIC_PUBLISH)
                    msg_req->setTopicMsg(msg);
                auto tmp = std::dynamic_pointer_cast<BaseMessage>(msg_req);
                BaseMessage::Ptr msg_rsp;
                bool ret = _requestor->send(con,tmp,msg_rsp);
                if(ret == false)
                {
                    ELOG("主题操作失败");
                    return false;
                }
                auto top_rsp = std::dynamic_pointer_cast<TopicResponse>(msg_rsp);
                if(top_rsp.get() == nullptr)
                {
                    ELOG("转换失败%s",strerror(errno));
                    return false;
                }
                if(top_rsp->rCode() != RCode::RCODE_OK)
                {
                    ELOG("topic请求出错%s",errorCode(top_rsp->rCode()).c_str());
                    return false;
                }
                ILOG("主题操作成功");
                return true;
            }
        private:
            std::mutex _mtx;
            Requestor::Ptr _requestor; // 发送消息
            std::unordered_map<std::string,SubCallBack> _topic_callbacks; // 主题和回调的映射关系
        };
    }
}