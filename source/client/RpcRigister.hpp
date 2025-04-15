// 客户端的服务的注册和服务的发现

#pragma once

#include "requestor.hpp"
#include <unordered_set>
namespace rpcframe
{
    namespace client
    {
        // 服务提供者
        class Provider
        {
        public:
            using Ptr = std::shared_ptr<Provider>;
            Provider(const Requestor::Ptr& requestor):_requestor(requestor)
            {}
            bool rigistMethod(const BaseConnection::Ptr& con,const std::string& method,const Address_t& addr)
            {
                // 构造请求
                auto msg = MessageFactory::create<ServiceRequest>();
                msg->setId(UUIDTool::getUUID());
                msg->setMtype(Mtype::REQ_SERVICE);
                msg->setMethod(method);
                msg->setServiceOptType(ServiceOpType::SERVICE_REGISTY);
                msg->setAddress(addr.first,addr.second);
                auto tmp = std::dynamic_pointer_cast<BaseMessage>(msg);
                BaseMessage::Ptr msg_rsp;
                bool ret = _requestor->send(con,tmp,msg_rsp);
                if(ret == false)
                {
                    ELOG("%s注册失败",method.c_str());
                    return false;
                }
                auto service_rsp = std::dynamic_pointer_cast<ServiceResponse>(msg_rsp);
                if(service_rsp.get() == nullptr)
                {
                    // 
                    ELOG("响应转换失败");
                    return false;
                }
                if(service_rsp->rCode() != RCode::RCODE_OK)
                {
                    ELOG("服务注册失败:%s",errorCode(service_rsp->rCode()).c_str());
                    return false;
                }
                return true;
            }
        private:
            Requestor::Ptr _requestor;// 构造请求对象
        };

        // 这个类的作用帮助我们进行RR轮转
        class MethodHost
        {
        public:
            // 完美转发提高效率
            using Ptr = std::shared_ptr<MethodHost>;
            MethodHost(const std::vector<Address_t>& hosts):
                _hosts(hosts.begin(),hosts.end()),_index(0)
            {}
            void appendHost(const Address_t& addr) { 
                std::unique_lock<std::mutex> lock(_mtx);
                _hosts.push_back(addr);
            }
            void delHost(const Address_t& addr) {
                std::unique_lock<std::mutex> lock(_mtx);
                for(int i = 0;i < _hosts.size();i++)
                {
                    if(addr == _hosts[i])
                    {
                        _hosts.erase(_hosts.begin() + i);
                        return;
                    }
                }
            }

            bool empty() { return _hosts.empty(); }

            Address_t chooseAddess()
            {
                std::unique_lock<std::mutex> lock(_mtx);
                int ret = _index;
                _index = (_index + 1) % _hosts.size();
                return _hosts[ret];
            }
        private:
            std::mutex _mtx;
            size_t _index;
            std::vector<Address_t> _hosts;
        };
        // 服务的发现者
        class Discoverier
        {
        public:
            using OfflineCallBack = std::function<void(const Address_t&)>;
            using Ptr = std::shared_ptr<Discoverier>;

            Discoverier(const Requestor::Ptr& requestor,const OfflineCallBack& offcb):_requestor(requestor),
                _offline_callback(offcb)
            {}
        // RR轮转
            
            // 通过方法想要获取对应的主机信息，内部采用RR轮转,
            bool serviceDiscovery(const BaseConnection::Ptr& con,const std::string& method,Address_t& host)
            {
                // 可以直接去本地查找
                // 如果为空,就去构建发现请求
                std::unique_lock<std::mutex> lock(_mtx);
                if(_method_hosts.count(method) == 0) 
                {
                    // 组织请求今夕你给服务发现
                    auto msg = MessageFactory::create<ServiceRequest>();
                    msg->setId(UUIDTool::getUUID());
                    msg->setMtype(Mtype::REQ_SERVICE);
                    msg->setMethod(method);
                    msg->setServiceOptType(ServiceOpType::SERVICE_DISCOVERY);
                    BaseMessage::Ptr msg_rsp;
                    auto tmp = std::dynamic_pointer_cast<BaseMessage>(msg);
                    bool ret = _requestor->send(con,tmp,msg_rsp);
                    if(ret == false)
                    {
                        ELOG("服务发现失败");
                        return false;
                    }
                    ELOG("获取信息成功");
                    auto service_rsp = std::dynamic_pointer_cast<ServiceResponse>(msg_rsp);
                    if(service_rsp.get() == nullptr)
                    {
                        ELOG("响应类型转换失败");
                        return false;
                    }
                    if(service_rsp->rCode() != RCode::RCODE_OK)
                    {
                        ELOG("服务发现失败%s",errorCode(service_rsp->rCode()).c_str());
                        return false;
                    }
                    // 注册发现的新的主机

                    auto method_hosts = std::make_shared<MethodHost>(service_rsp->Hosts());
                    _method_hosts[method] = method_hosts;
                    if(method_hosts->empty())
                    {
                        ELOG("服务发现失败，啥也没有");
                        return false;
                    }
                    host = method_hosts->chooseAddess();
                    return true;
                }

                host = _method_hosts[method]->chooseAddess();
                return true;
            }
            // dispather调用
            void onServiceRequstor(const BaseConnection::Ptr& con,const ServiceRequest::Ptr& msg)
            {
                // 判断是上线还是下线
                // 上线请求
                std::unique_lock<std::mutex> lock(_mtx);
                auto optype = msg->serviceOptType();
                if(optype == ServiceOpType::SERVICE_ONLINE)
                {
                    auto it = _method_hosts.find(msg->method());
                    if(it == _method_hosts.end())
                    {
                        auto method_host = std::make_shared<MethodHost>(std::vector<Address_t>());
                        method_host->appendHost(msg->address());
                        _method_hosts[msg->method()] = method_host;
                    }else _method_hosts[msg->method()]->appendHost(msg->address());

                }
                else if(optype == ServiceOpType::SERVICE_OFFLINE)
                {
                    // 下线同志
                    auto it = _method_hosts.find(msg->method());
                    if(it == _method_hosts.end()) return;
                    _method_hosts[msg->method()]->delHost(msg->address());
                    _offline_callback(msg->address());
                }
                else 
                {
                    // 
                    ELOG("未知的请求");
                }
            }
        private:
            OfflineCallBack _offline_callback;// 下线回调
            std::mutex _mtx;
            // hash<method,vector<host>>
            std::unordered_map<std::string,MethodHost::Ptr> _method_hosts;
            Requestor::Ptr _requestor;
        };
    };
}