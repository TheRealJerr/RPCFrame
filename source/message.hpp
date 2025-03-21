#pragma once
#include "detail.hpp"
#include "abstract.hpp"
#include <jsoncpp/json/json.h>
#include "fields.hpp"
#include <vector>

namespace rpcframe
{
    class JsonMessage : public BaseMessage
    {
    public:
        using Ptr = std::shared_ptr<JsonMessage>;

        virtual std::string serialize() override
        {
            std::string ret;
            bool n = JsonTools::serialize(_body, ret);
            if (n == false)
                return "";
            else
                return ret;
        }

        virtual bool unserialize(const std::string &msg) override
        {
            return JsonTools::deserialize(_body, msg);
        }

        const Json::Value &body() const { return _body; }

        Json::Value &body() { return _body; }

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
            if (body()[KEY_RCODE].isNull())
            {
                ELOG("响应中没有状态码!");
                return false;
            }
            // 判断字段类型是否正确
            if (body()[KEY_RCODE].isIntegral() == false)
            {
                ELOG("相应状态码类型错误!");
                return false;
            }
            return true;
        }
        virtual RCode rCode() const 
        {
            return (RCode)body()[KEY_RCODE].asInt();
        }

        virtual void setRCode(RCode rc)
        {
            body()[KEY_RCODE] = (int)rc;
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
            if (body()[KEY_METHOD].isNull() ||
                !body()[KEY_METHOD].isString())
            {
                ELOG("Rpc请求中没有方法名称或者类型错误!");
                return false;
            }
            // 判断字段类型是否正确
            if (body()[KEY_PARAMS].isNull() ||
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
        void setMethod(const std::string &msg)
        {
            body()[KEY_METHOD] = msg;
        }
        // 获取参数
        Json::Value params() const { return body()[KEY_PARAMS]; }
        // 设置参数
        void setParams(const Json::Value &_args) { body()[KEY_PARAMS] = _args; }
    };

    class TopicRequest : public JsonRequest
    {
    public:
        using Ptr = std::shared_ptr<TopicRequest>;
        virtual bool check() const override
        {
            // rpc请求中请求方法名称
            // 1.
            if (body()[KEY_TOP_KEY].isNull() ||
                !body()[KEY_TOP_KEY].isString())
            {
                ELOG("Rpc请求中没有主题名称或者类型错误!");
                return false;
            }
            // 判断字段类型是否正确
            if (body()[KEY_OP_TYPE].isNull() ||
                !body()[KEY_OP_TYPE].isIntegral())
            {
                ELOG("主题请求中没有操作类型或者类型错误");
                return false;
            }
            // 发布的时候才会进行消息检测
            if (body()[KEY_OP_TYPE].asInt() == (int)TopicOptType::TOPIC_PUBLISH &&
                (body()[KEY_TOP_MSG].isNull() == true ||
                 body()[KEY_TOP_MSG].isString() == false))
            {
                // 如果他是一个发布消息的字段
                ELOG("主题消息发布请求中没有消息内容字段或者内容错误");
                return false;
            }

            return true;
        }

        std::string topicKey() const
        {
            return body()[KEY_TOP_KEY].asString();
        }
        void setTopicKey(const std::string &msg)
        {
            body()[KEY_TOP_KEY] = msg;
        }

        TopicOptType topicOpType() const
        {
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

        void setTopicMsg(const std::string &msg)
        {
            body()[KEY_TOP_MSG] = msg;
        }
    };
    
    using Address_t = std::pair<std::string,int>;
    class ServiceRequest : public JsonRequest
    {
    public:
        using Ptr = std::shared_ptr<ServiceRequest>;
        bool check() const override
        {
            if (body()[KEY_METHOD].isNull() ||
                !body()[KEY_METHOD].isString())
            {
                ELOG("服务请求中没有方法名称或者类型错误");
                return false;
            }
            // 判断字段类型是否正确
            if (body()[KEY_OP_TYPE].isNull() ||
                !body()[KEY_OP_TYPE].isIntegral())
            {
                ELOG("服务请求中没有操作类型或者类型错误");
                return false;
            }
            // debug -- 服务发现没有host字段
            if(body()[KEY_OP_TYPE].asInt() != (int)ServiceOpType::SERVICE_DISCOVERY && 
                (body()[KEY_HOST].isNull() || 
                !body()[KEY_HOST].isObject() || 
                body()[KEY_HOST][KEY_HOST_IP].isNull() ||
                !body()[KEY_HOST][KEY_HOST_IP].isString() ||
                body()[KEY_HOST][KEY_HOST_PORT].isNull() || 
                !body()[KEY_HOST][KEY_HOST_PORT].isIntegral()))
            {
                ELOG("服务主机地址信息错误,可能由多方面引起...");
            }
            return true;
        }

        // 获取方法
        std::string method() const { return body()[KEY_METHOD].asString(); }
        // 设置接口
        void setMethod(const std::string &msg)
        {

            body()[KEY_METHOD] = msg;
        }

        ServiceOpType serviceOptType() const { return (ServiceOpType)(body()[KEY_OP_TYPE]).asInt(); }

        void setServiceOptType(ServiceOpType opt)
        {
            body()[KEY_OP_TYPE] = (int)opt;
        }

        Address_t address() const 
        {
            Address_t ret;
            ret.first = body()[KEY_HOST][KEY_HOST_IP].asString();
            ret.second = body()[KEY_HOST][KEY_HOST_PORT].asInt();
            return ret;
        }

        void setAddress(const std::string& ip,int port)
        {
            // body()[KEY_HOST][KEY_HOST_IP] = ip;
            // body()[KEY_HOST][KEY_HOST_PORT] = port;
            Json::Value val;
            val[KEY_HOST_IP] = ip;
            val[KEY_HOST_PORT] = port;
            body()[KEY_HOST] = val;
        }

    };
}

namespace rpcframe
{
    class RpcResponse : public JsonResponse
    {
    public:
        using Ptr = std::shared_ptr<RpcResponse>;
        virtual bool check() const override
        {
            //if(JsonResponse::check() == false) return false;
            if (body()[KEY_RCODE].isNull())
            {
                ELOG("Rpc响应中没有状态码!");
                return false;
            }
            // 判断字段类型是否正确
            if (body()[KEY_RCODE].isIntegral() == false)
            {
                ELOG("Rpc相应状态码类型错误!");
                return false;
            }
            if(body()[KEY_RESULT].isNull() || 
                body()[KEY_RESULT].isObject() == false)
            {
                ELOG("响应中没有结果");
                return false;
            }
            return true;
        }

        

        Json::Value result() const 
        {
            return body()[KEY_RESULT];
        }
        void setResult(const Json::Value& rsl)
        {
            body()[KEY_RESULT] = rsl;
        }
    private:
    };

    class TopicResponse : public JsonResponse
    {
    public:
        using Ptr = std::shared_ptr<TopicResponse>;
        virtual bool check() const override
        {
            //if(JsonResponse::check() == false) return false;
            if (body()[KEY_RCODE].isNull())
            {
                ELOG("Topic响应中没有状态码!");
                return false;
            }
            // 判断字段类型是否正确
            if (body()[KEY_RCODE].isIntegral() == false)
            {
                ELOG("Topic相应状态码类型错误!");
                return false;
            }
            
            return true;
        }

        // RCode rCode() const 
        // {
        //     return (RCode)body()[KEY_RCODE].asInt();
        // }

        // void setRCode(RCode rc)
        // {
        //     body()[KEY_RCODE] = (int)rc;
        // }

        // Json::Value result() const 
        // {
        //     return body()[KEY_RESULT];
        // }
        // void setResult(const Json::Value& rsl)
        // {
        //     body()[KEY_RESULT] = rsl;
        // }
    private:
    };
    class ServiceResponse : public JsonResponse
    {
    public:
        using Ptr = std::shared_ptr<ServiceResponse>;
        virtual bool check() const override
        {
            //if(JsonResponse::check() == false) return false;
            if (body()[KEY_RCODE].isNull())
            {
                ELOG("Rpc响应中没有状态码!");
                return false;
            }
            // 判断字段类型是否正确
            if (body()[KEY_RCODE].isIntegral() == false)
            {
                ELOG("Rpc相应状态码类型错误!");
                return false;
            }

            if(body()[KEY_OP_TYPE].isNull() ||
                body()[KEY_OP_TYPE].isIntegral() == false)
            {
                ELOG("没有类型操作或者类型操作失败");
                return false;
            }

            if((ServiceOpType)body()[KEY_OP_TYPE].asInt() == ServiceOpType::SERVICE_DISCOVERY && 
                (body()[KEY_METHOD].isNull() || 
                (body()[KEY_METHOD].isString() == false || 
                body()[KEY_HOST].isNull() || 
                body()[KEY_HOST].isArray() == false)))
            {
                ELOG("服务响应中响应字段错误");
                return false;
            }

            if(body()[KEY_RESULT].isNull() || 
                body()[KEY_RESULT].isObject() == false)
            {
                ELOG("响应中没有结果");
                return false;
            }
            return true;
        }

        // RCode rCode() const 
        // {
        //     return (RCode)body()[KEY_RCODE].asInt();
        // }

        // void setRCode(RCode rc)
        // {
        //     body()[KEY_RCODE] = (int)rc;
        // }

        std::string method() const 
        {
            return body()[KEY_METHOD].asString();
        }

        void setMethod(const std::string& method)
        {
            body()[KEY_METHOD] = method;
        }
        void setHost(const std::vector<Address_t>& addrs)
        {
            for(auto& addr : addrs)
            {
                Json::Value val;
                val[KEY_HOST_IP] = addr.first;
                val[KEY_HOST_PORT] = addr.second;
                body()[KEY_HOST].append(val);
            }
            // 
        }
        std::vector<Address_t> Hosts() const 
        {
            std::vector<Address_t> addrs;
            int len = body()[KEY_HOST].size();
            for(int i = 0;i < len;i++)
                addrs.push_back({body()[KEY_HOST][i][KEY_HOST_IP].asString(),body()[KEY_HOST][i][KEY_HOST_PORT].asInt()});
            return addrs;
        }
    private:
    };
    
    // 构建一个request, response构建工厂

    // 构建消息对象的生产工厂

    class MessageFactory
    {
    public:
        static BaseMessage::Ptr create(Mtype type)
        {
            switch (type)
            {
                case Mtype::REQ_RPC:
                    return std::make_shared<RpcRequest>();
                case Mtype::RSP_RPC:
                    return std::make_shared<RpcResponse>();
                case Mtype::REQ_TOP:
                    return std::make_shared<TopicRequest>();
                case Mtype::RSP_TOP:
                    return std::make_shared<TopicResponse>();
                case Mtype::REQ_SERVICE:
                    return std::make_shared<ServiceRequest>();
                case Mtype::RSP_SERVICE:
                    return std::make_shared<ServiceResponse>();
            }
            return nullptr;
        }
    };
}