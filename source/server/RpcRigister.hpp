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

            PDManager() {}
            void onServiceRequest(const BaseConnection::Ptr &con, const ServiceRequest::Ptr &msg)
            {
            }

            void onConnectionShutDown(const BaseConnection::Ptr &con)
            {
            }

        private:
            ProviderManager::Ptr _providers;
            DiscoverManager::Ptr _discoverys;
        };
    }
}