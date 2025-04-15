// 对Rpc消息进行派发的模块
#pragma once
#pragma once
#include <iostream>
#include "../Common/detail.hpp"
#include "../Common/message.hpp"
#include "../Common/net.hpp"
#include <vector>

namespace rpcframe
{
    namespace server
    {
        // 定义服务的描述
        enum class VType
        {
            INTEGRAL = 0, // integral
            NUMRIC,
            STRING,
            ARRAY,
            OBJECT,
            BOOL,
        };

        // 构建好了不能修改
        class ServiceDescribe
        {
        public:
            using Ptr = std::shared_ptr<ServiceDescribe>;
            // 定义业务的回调函
            using ServiceCallBack = std::function<void(const Json::Value &, Json::Value &)>;
            using ParamsDescribe = std::pair<std::string, VType>;
            
            ServiceDescribe(const std::string& method_name,const std::vector<ParamsDescribe>& describe,VType rtype,const ServiceCallBack& callback)
                :_method(method_name),_paras_check(std::move(describe)),_ret_type(rtype),_call_bak(std::move(callback))
            {}
            bool ParamCheck(const Json::Value &val)
            {
                // 针对收到req中的参数进行校验
                // 对val进行参数校验
                // 判断描述的参数字段是否存在,类型是否一致
                for (auto &descri : _paras_check)
                {
                    if (!val.isMember(descri.first))
                    {
                        std::string tmp;
                        JsonTools::serialize(val,tmp);
                        // std::cout <<  tmp << std::endl;
                        ELOG("字段缺失:%s", descri.first.c_str());
                        return false;
                    }
                    if (check(descri.second, val[descri.first]) == false)
                    {
                        ELOG("参数类型不匹配");
                        return false;
                    }
                }
                // 校验成功

                return true;
            }

            const std::string& Method() const { return _method; }

            bool call(const Json::Value& params,Json::Value& result)
            {
                _call_bak(params,result);
                if(rTypeCheck(result) == false) 
                {
                    ELOG("服务的回调函数返回的类型错误");
                    return false;
                }
                return true;

            }
            bool rTypeCheck(const Json::Value& ret)
            {
                return check(_ret_type,ret);
            }
        private:
            bool check(VType type, const Json::Value &val)
            {
                switch (type)
                {
                case VType::BOOL:
                    return val.isBool();
                case VType::INTEGRAL:
                    return val.isIntegral();
                case VType::NUMRIC:
                    return val.isNumeric();
                case VType::STRING:
                    return val.isString();
                case VType::ARRAY:
                    return val.isArray();
                case VType::OBJECT:
                    return val.isObject();
                }
                return false; // 未定义的字段
            }

        private:
            std::string _method;
            ServiceCallBack _call_bak;
            // 需要对参数的描述
            /*
            num1 = int(1) , num2 = int(2) , result =
            */
            std::vector<ParamsDescribe> _paras_check; // 参数校验
            // we need describe link
            // num1 , int
            // num2 , int 来上线新的功能
            VType _ret_type; // 结果的描述
        };

        // 通过建造者模式进行构建
        class ServiceFactory
        {
        public:
            // static ServiceDescribe::Ptr create(); // 构建描述信息交由内部进行管理
            void setRetType(VType type) { _ret_type = type; }
            void setParams(const std::string& name,VType type)
            {
                _paras_check.push_back({name,type});
            }
            void setCallBack(const ServiceDescribe::ServiceCallBack& call_back) 
            {
                _call_bak = call_back;
            }
            void setMethod(const std::string& method) { _method = method; }

            // 基于建造者模式进行构建
            ServiceDescribe::Ptr build() 
            {
                return std::make_shared<ServiceDescribe>(_method,_paras_check,_ret_type,_call_bak);
            }
        private:
            std::string _method;
            ServiceDescribe::ServiceCallBack _call_bak;
            std::vector<ServiceDescribe::ParamsDescribe> _paras_check; // 参数校验
            VType _ret_type; // 结果的描述
        };
        // 服务管理管理
        class ServiceManager
        {
        public:
            using Ptr = std::shared_ptr<ServiceManager>;

            void insert(const ServiceDescribe::Ptr& new_service)
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _services.insert(std::make_pair(new_service->Method(),new_service));
            }
            
            ServiceDescribe::Ptr select(const std::string& name)
            {
                // 进行查找
                std::unique_lock<std::mutex> lock(_mtx);
                auto it = _services.find(name);
                if(it == _services.end()) return nullptr; // 空的对象 没有对应的操作
                else return it->second;
            }

            void remove(const std::string& name)
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _services.erase(name);
            }
        private:
            std::mutex _mtx;
            std::unordered_map<std::string, ServiceDescribe::Ptr> _services;
        };
        class RpcRouter
        {
        public:
            using Ptr = std::shared_ptr<RpcRouter>;
            // 注册到dispatcher针对rpc请求进行回调处理的业务函数
            // constructor
            RpcRouter() : _service_manager(std::make_shared<ServiceManager>())
            {}

            void onRpcRequest(const BaseConnection::Ptr &con,RpcRequest::Ptr &req)
            {
                
                // 1. 判断当前服务端能否提供服务
                auto service = _service_manager->select(req->method());
                if(service.get() == nullptr)
                {
                    ELOG("无法提供%s服务",req->method().c_str());
                    return response(con,req,Json::Value(),rpcframe::RCode::RCODE_NOT_FIND_SEVIVE);
                }
                // 2. 提供服务检验
                // 
                if(service->ParamCheck(req->params()) == false)
                {
                    ELOG("参数校验失败");
                    return response(con,req,Json::Value(),rpcframe::RCode::RCODE_NOT_FIND_SEVIVE);
                }

                Json::Value result;
                bool ret = service->call(req->params(),result);
                if(ret == false)
                {
                    ELOG("服务端内部错误,返回的类型失败");
                    return response(con,req,Json::Value(),rpcframe::RCode::RCODE_INTERNAL_ERR);
                }
                return response(con,req,result,rpcframe::RCode::RCODE_OK);
            }

            // 注册服务
            void rigisterMethod(const ServiceDescribe::Ptr & new_service) 
            {
                _service_manager->insert(new_service);
            }
        private:
            void response(const rpcframe::BaseConnection::Ptr& conn,rpcframe::RpcRequest::Ptr& msg,const Json::Value& rsp,rpcframe::RCode rcode)
            {
                RpcResponse::Ptr response = rpcframe::MessageFactory::create<rpcframe::RpcResponse>();
                response->setId(msg->rid());
                response->setMtype(rpcframe::Mtype::RSP_RPC);
                response->setRCode(rcode);
                response->setResult(rsp);
                conn->send(response);
            }
        private:
            ServiceManager::Ptr _service_manager; // 注册
        };
    }
}