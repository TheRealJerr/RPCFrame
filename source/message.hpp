#pragma once
#include "detail.hpp"
#include "abstract.hpp" 
#include <jsoncpp/json/json.h>
#include "fields.hpp"
namespace rpcframe
{
    class JsonMessage : public BaseMessage
    {
    public:
        using Ptr = std::shared_ptr<JsonMessage>;

        virtual std::string serialize() override
        {
            std::string ret;
            bool n = JsonTools::serialize(_body,ret);
            if(n == false)  return "";
            else return ret;
        }

        virtual bool unserialize(const std::string& msg) override
        {
            return JsonTools::deserialize(_body,msg);
        }

        const Json::Value& body() const { return _body; }

        Json::Value& body() { return _body; }

    private:
        Json::Value _body;
    };

    class JsonRequest : public JsonMessage
    {
    public:
        using Ptr = std::shared_ptr<JsonMessage>;
    private:

    };

    class JsonResponse : public JsonMessage
    {
    public:
        using Ptr = std::shared_ptr<JsonResponse>;
        virtual bool check() const override
        {
            // 在相应中,相应状态吗
            // 检查响应状态吗即可,类型是否正确
            // 判断字段是否存在
            if(body()[KEY_RCODE].isNull())
            {
                ELOG("响应中没有状态码!");
                return false;
            }
            // 判断字段类型是否正确
            if(body()[KEY_RCODE].isIntegral() == false)
            {
                ELOG("相应状态码类型错误!");
                return false;
            }
            return true;
        }
    private:
    };

    class RpcRequest : public JsonRequest
    {
    public:
        using Ptr = std::shared_ptr<RpcRequest>;
        virtual bool check() const override
        {
            // rpc请求中请求方法名称
            // 1. 
            if(body()[KEY_METHOD].isNull() || 
                !body()[KEY_METHOD].isNull())
            {
                ELOG("Rpc请求中没有方法名称或者类型错误!");
                return false;
            }
            // 判断字段类型是否正确
            if(body()[KEY_PARAMS].isNull() || 
                !body()[KEY_PARAMS].isObject())
            {
                ELOG("没有参数信息或参数信息类型错误");
                return false;
            }
            return true;
        }
        // 获取方法
        std::string method() const { return body()[KEY_METHOD].asString(); }
        // 设置接口
        void setMethod(const std::string& msg) { 
            body()[KEY_METHOD] = msg;
        }
        // 获取参数
        Json::Value params() const { return body()[KEY_PARAMS]; }
        // 设置参数
        void setParams(const Json::Value& _args) { body()[KEY_PARAMS] = _args; }
    };

    class TopicRequest : public JsonRequest
    {
    public:
        using Ptr = std::shared_ptr<TopicRequest>;
        virtual bool check() const override
        {
            // rpc请求中请求方法名称
            // 1. 
            if(body()[KEY_TOP_KEY].isNull() || 
                !body()[KEY_TOP_KEY].isString())
            {
                ELOG("Rpc请求中没有主题名称或者类型错误!");
                return false;
            }
            // 判断字段类型是否正确
            if(body()[KEY_OP_TYPE].isNull() || 
                !body()[KEY_OP_TYPE].isIntegral())
            {
                ELOG("主题请求中没有操作类型或者类型错误");
                return false;
            }

            if(body()[KEY_OP_TYPE].asInt() == (int)TopicOptType::TOPIC_PUBLISH && 
                (body()[KEY_TOP_MSG].isNull() == true || 
                body()[KEY_TOP_MSG].isString() == false))
            {
                // 如果他是一个发布消息的字段
                ELOG("主题消息发布请求中没有消息内容字段或者内容错误");
                return false;
            }

            return true;
        }

        std::string topicKey() const {
            return body()[KEY_TOP_KEY].asString();
        }
        void setTopicKey(const std::string& msg) { 
            body()[KEY_TOP_KEY] = msg;
        }

        TopicOptType topicOpType() const { 
            return (TopicOptType)(body()[KEY_OP_TYPE].asInt());
        }

        void setTopicOptType(TopicOptType opt)
        {
            body()[KEY_OP_TYPE] = (int)opt;
        }

        std::string topicMsg() const
        {
            return (body()[KEY_TOP_MSG].asString());
        }

        void setTopicMsg(const std::string& msg) 
        {
            body()[KEY_TOP_MSG] = msg;
        }
    };


    class ServiceRequest : public JsonRequest
    {
    public:
        bool check() const override
        {
            
        }
    };
}
