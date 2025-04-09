#pragma once
#include <iostream>
#include "../Common/detail.hpp"
#include "../Common/message.hpp"
#include "../Common/net.hpp"
#include <vector>
#include <set>

namespace rpcframe
{
    namespace server
    {
        // 提供者
        class ProviderManager
        {
        public:
            using Ptr = std::shared_ptr<ProviderManager>;
            struct Provider
            {
                std::mutex mtx_;
                BaseConnection::Ptr _con; // 连接
                using Ptr = std::shared_ptr<Provider>;
                // 注意这里一定是外部能够访问的地址而不是监听的地址
                std::vector<std::string> _mothods;
                Address_t _host;

                void appendMethod(const std::string &method)
                {
                    std::unique_lock<std::mutex> lock(mtx_);
                    _mothods.push_back(method);
                }

                Provider(const BaseConnection::Ptr con, const Address_t &host) : _con(con), _host(host)
                {
                }
            };
            // 当新的服务器注册新的的服务的时候进行调用
            // 可以是0->1 也可以是 k -> k+1
            Provider::Ptr addProvider(const BaseConnection::Ptr con, const Address_t &host, const std::string &method)
            {
                Provider::Ptr provider;
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    auto it = _connections.find(con);
                    if (it != _connections.end())
                    {
                        provider = _connections[con];
                    }
                    else
                    {
                        provider = std::make_shared<Provider>(con, host);
                        _connections.insert({con, provider});
                    }
                    // 方法 -> 主机映射添加
                    auto &method_providers = _providers[method];
                    method_providers.insert(provider);
                }
                // 得到了provider对象
                provider->appendMethod(method); // 对应的主机添加了新的方法名称
                return provider;
            }
            // 服务提供者下线的时候进行通知
            void delProvider(const BaseConnection::Ptr &con)
            {
                std::unique_lock<std::mutex> lock(_mtx);
                auto it = _connections.find(con);
                if (it != _connections.end())
                {
                    // 找到provider提供的method
                    for (auto &method : it->second->_mothods)
                    {
                        // 删除每个方法的provider
                        _providers[method].erase(it->second);
                    }
                    _connections.erase(con);
                }
                else
                {
                    return; // 没找到直接退出
                }
                // 方法 -> 主机映射添加
            }
            // 当断开连接的时候，获取他的信息，进行通知
            std::vector<Address_t> methodHosts(const std::string& method)
            {
                // 得到对应的d
                std::unique_lock<std::mutex> lock(_mtx);
                auto it = _providers.find(method);
                if(it == _providers.end())
                    return {};
                std::vector<Address_t> ret;
                for(auto& provider : it->second)
                {
                    ret.push_back(provider->_host);
                }
                return ret;
            }
            Provider::Ptr getProvider(const BaseConnection::Ptr &con)
            {
                std::unique_lock<std::mutex> lock(_mtx);
                auto it = _connections.find(con);
                if (it != _connections.end())
                {
                    return it->second;
                }
                return nullptr;
            }

        private:
            std::mutex _mtx;
            // 服务 -> 主机映射
            std::unordered_map<std::string, std::set<Provider::Ptr>> _providers;
            // 连接 -> 主机映射
            std::unordered_map<BaseConnection::Ptr, Provider::Ptr> _connections;
        };
        // 服务发现者的管理
        class DiscoverManager
        {
        public:
            using Ptr = std::shared_ptr<DiscoverManager>;
            // 1. 客户端连接
            // 2. 记录客户端 -- 服务的映射
            struct Discovery
            {
                std::mutex mtx_;
                using Ptr = std::shared_ptr<Discovery>;
                BaseConnection::Ptr _con; // 连接
                using Ptr = std::shared_ptr<Discovery>;
                // 注意这里一定是外部能够访问的地址而不是监听的地址
                std::vector<std::string> _mothods; // 发现过的服务的名称
                void appendMethod(const std::string &method)
                {
                    std::unique_lock<std::mutex> lock(mtx_);
                    _mothods.push_back(method);
                }

                Discovery(const BaseConnection::Ptr &con) : _con(con)
                {
                }
            };
            // 发现新的功能
            Discovery::Ptr addDiscovery(const BaseConnection::Ptr &con, const std::string &method)
            {
                // 连接->发现
                Discovery::Ptr discovery;
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    auto it = _connections.find(con);
                    if (it == _connections.end())
                    {
                        // 注册新的发现
                        discovery = std::make_shared<Discovery>(con);
                        _connections.insert({con, discovery});
                    }
                    else
                        discovery = it->second;
                    // 获取到了连接
                    _discoveriers[method].insert(discovery);
                }
                discovery->appendMethod(method);
                return discovery;
            }

            void delDiscovery(const BaseConnection::Ptr &con)
            {
                // 删除链接
                std::unique_lock<std::mutex> lock(_mtx);
                auto it = _connections.find(con);
                if(it == _connections.end()) return;
                // 
                // 针对所有方法删除
                for(auto& method : it->second->_mothods)
                {
                    _discoveriers[method].erase(it->second);
                }
                _connections.erase(con);
            }
            void onlineNotify(const std::string &method,const Address_t& host)
            {
                notify(method,host,ServiceOpType::SERVICE_ONLINE);
            }

            void offlineNotify(const std::string &method,const Address_t& host)
            {
                notify(method,host,ServiceOpType::SERVICE_OFFLINE);
            }
        private:
            // 定义上线/下线提醒
            void notify(const std::string &method,const Address_t& host,ServiceOpType optype)
            {
                std::unique_lock<std::mutex> lock(_mtx);
                if(_discoveriers.count(method) == false) return;
                // 向所有对应主机发送请求
                // 创建消息
                auto msg = MessageFactory::create<ServiceRequest>();
                msg->setId(UUIDTool::getUUID());
                msg->setMtype(Mtype::REQ_SERVICE);
                msg->setMethod(method);
                msg->setServiceOptType(optype);
                msg->setAddress(host.first,host.second);
                // 
                for(auto& discovery : _discoveriers[method])
                    discovery->_con->send(msg);
            }
        private:
            std::mutex _mtx;
            std::unordered_map<std::string, std::set<Discovery::Ptr>> _discoveriers;
            std::unordered_map<BaseConnection::Ptr, Discovery::Ptr> _connections;
        };

        class PDManager
        {
        public:
            using Ptr = std::shared_ptr<PDManager>;

            PDManager():
                _providers(std::make_shared<ProviderManager>()),
                _discoverys(std::make_shared<DiscoverManager>())
            {}
            void onServiceRequest(const BaseConnection::Ptr &con, const ServiceRequest::Ptr &msg)
            {
                ServiceOpType opt_type = msg->serviceOptType();
                // 服务操作请求,服务注册/服务发现
                // 1. 新增服务提供者, 进行服务发现的通知
                if(opt_type == ServiceOpType::SERVICE_REGISTY)
                {
                    _providers->addProvider(con,msg->address(),msg->method());
                    _discoverys->onlineNotify(msg->method(),msg->address()); // 上限通知
                    return rigisterResponse(con,msg);
                }
                // 2. 新增的服务发现
                else if(opt_type == ServiceOpType::SERVICE_DISCOVERY)
                {
                    _discoverys->addDiscovery(con,msg->method());
                    return discoverResponse(con,msg);
                }
                else
                {
                    // 错误的消息
                    errorResponse(con,msg);
                    ELOG("收到服务操作请求,操作类型错误");
                }
            }

            void onConnectionShutDown(const BaseConnection::Ptr &con)
            {
                // 连接关闭的回调函数
                auto provider = _providers->getProvider(con);
                if(provider.get())
                {
                    // 他就是一个服务提供者
                    for(auto& method : provider->_mothods)
                    {
                        _discoverys->offlineNotify(method,provider->_host);
                    }
                    _providers->delProvider(con);
                }
                
                _discoverys->delDiscovery(con);
            }
        private:
            void rigisterResponse(const BaseConnection::Ptr &con, const ServiceRequest::Ptr &msg)
            {
                auto msg_rsp = MessageFactory::create<ServiceResponse>();
                msg_rsp->setId(msg->rid());
                msg_rsp->setMtype(Mtype::RSP_SERVICE);
                msg_rsp->setRCode(RCode::RCODE_OK);
                msg_rsp->setServiceOpType(ServiceOpType::SERVICE_REGISTY);
                con->send(msg_rsp);
            }

            void discoverResponse(const BaseConnection::Ptr &con, const ServiceRequest::Ptr &msg)
            {

                auto msg_rsp = MessageFactory::create<ServiceResponse>();
                msg_rsp->setId(msg->rid());
                msg_rsp->setMtype(Mtype::RSP_SERVICE);
                msg_rsp->setServiceOpType(ServiceOpType::SERVICE_DISCOVERY);
                auto hosts = _providers->methodHosts(msg->method());
                if(hosts.empty())
                {
                    msg_rsp->setRCode(RCode::RCODE_NOT_FIND_SEVIVE);
                    return con->send(msg_rsp);
                }
                msg_rsp->setRCode(RCode::RCODE_OK);
                msg_rsp->setMethod(msg->method());
                msg_rsp->setHost(hosts);
                con->send(msg_rsp);
            }

            void errorResponse(const BaseConnection::Ptr& con,const ServiceRequest::Ptr& msg)
            {
                auto msg_rsp = MessageFactory::create<ServiceResponse>();
                msg_rsp->setId(msg->rid());
                msg_rsp->setMtype(Mtype::RSP_SERVICE);
                msg_rsp->setRCode(RCode::RCODE_INVALID_MSG);
                msg_rsp->setServiceOpType(ServiceOpType::SERVICE_UNKOWN);
                con->send(msg_rsp);
            }
        private:
            ProviderManager::Ptr _providers;
            DiscoverManager::Ptr _discoverys;
        };
    }
}