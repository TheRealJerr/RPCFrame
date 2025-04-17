// 这里我们实现的是多种客户端的实现
#pragma once
#include "../Common/message.hpp"
#include "../Common/net.hpp"
#include "requestor.hpp"
#include "RpcCaller.hpp"
#include "../Common/dispatcher.hpp"
#include "RpcRigister.hpp"
#include "RpcTopic.hpp"

namespace rpcframe
{
    namespace client
    {
        // 服务登记客户端
        class RigClient
        {
        public:
            // 连接注册中心
            RigClient(const Address_t &rigaddr) : _requestor(std::make_shared<Requestor>()),
                                                  _dispatcher(std::make_shared<DisPatcher>()),
                                                  _provider(std::make_shared<Provider>(_requestor)),
                                                  _client(ClientFactory::create(rigaddr.first, rigaddr.second)),
                                                  _host(rigaddr)
            {
                auto cb = std::bind(&rpcframe::client::Requestor::onResponse, _requestor.get(), std::placeholders::_1, std::placeholders::_2);
                _dispatcher->rigisterHandler<rpcframe::BaseMessage>(rpcframe::Mtype::RSP_SERVICE, cb);
                _client->setMessageCallBack(std::bind(&rpcframe::DisPatcher::onMessage, _dispatcher.get(), std::placeholders::_1, std::placeholders::_2));
                _client->connect();
            }
            using Ptr = std::shared_ptr<RigClient>;

            // 向外提供的注册的接口
            // 这里一定要分清，我们云服务器bind的接口和我们外部连接的接口不同
            bool rigisteMethod(const std::string &method, const Address_t &addr)
            {
                return _provider->rigistMethod(_client->connection(), method, addr);
            }

        private:
            Requestor::Ptr _requestor; // 请求的管理
            DisPatcher::Ptr _dispatcher;
            Provider::Ptr _provider; // 服务的提供者
            BaseClient::Ptr _client;
            Address_t _host;
        };
        // 服务发现的客户端
        class DisClient
        {
        public:
            using Ptr = std::shared_ptr<DisClient>;
            DisClient(const Address_t &rigaddr,const Discoverier::OfflineCallBack& offcb) : _requestor(std::make_shared<Requestor>()),
                                                  _dispatcher(std::make_shared<DisPatcher>()),
                                                  _discoverier(std::make_shared<Discoverier>(_requestor,offcb)),
                                                  _host(rigaddr)
            {
                auto cb = std::bind(&rpcframe::client::Requestor::onResponse, _requestor.get(), std::placeholders::_1, std::placeholders::_2);
                _dispatcher->rigisterHandler<rpcframe::BaseMessage>(rpcframe::Mtype::RSP_SERVICE, cb);
                auto req_cb = std::bind(&client::Discoverier::onServiceRequstor, _discoverier.get(), std::placeholders::_1, std::placeholders::_2);
                _dispatcher->rigisterHandler<rpcframe::ServiceRequest>(rpcframe::Mtype::REQ_SERVICE, req_cb);
                _client = ClientFactory::create(rigaddr.first, rigaddr.second);
                _client->setMessageCallBack(std::bind(&rpcframe::DisPatcher::onMessage, _dispatcher.get(), std::placeholders::_1, std::placeholders::_2));
                
                _client->connect();
            }
            // 发现服务
            // 这里的addr是一个输出型参数
            bool findServer(const std::string &method, Address_t &addr)
            {
                // 构造发现请求
                return _discoverier->serviceDiscovery(_client->connection(), method, addr);
            }

        private:
            Requestor::Ptr _requestor; // 请求的管理
            DisPatcher::Ptr _dispatcher;
            Discoverier::Ptr _discoverier; // 服务的发现者
            BaseClient::Ptr _client;
            Address_t _host;
        };
        // 自定义hash方法
        // ip : port
        class AddressHash
        {
        public:
            size_t operator()(const Address_t& addr) const 
            {
                // 字符串拼接
                std::string str = addr.first + ":" + std::to_string(addr.second);
                return _hash(str);
            }
        private:
            std::hash<std::string> _hash;
        };
        // 目前我们还是短连接·· 2
        
        class RpcClient
        {
        public:
            RpcClient(bool enableDiscovery, const Address_t &rigaddr) :                                     // 是否启用服务发现功能
                                                                                                            // 同时也决定了rigaddr是服务提供者的地址 / 注册中心的地址进行发现
                                                                        _enable_discovery(enableDiscovery), // 是否进行服务的发现
                                                                        _requestor(std::make_shared<Requestor>()),
                                                                        _dispatcher(std::make_shared<DisPatcher>()),
                                                                        _caller(std::make_shared<RpcCaller>(_requestor))
            {
                auto cb = std::bind(&rpcframe::client::Requestor::onResponse, _requestor.get(), std::placeholders::_1, std::placeholders::_2);
                _dispatcher->rigisterHandler<rpcframe::BaseMessage>(rpcframe::Mtype::RSP_RPC, cb);

                // 构建连接
                if (_enable_discovery == true)
                {
                    // 构建上层处理回调
                    auto offcb = [this](const Address_t& addr)->void 
                    {
                        this->delClient(addr);
                    };
                    _dis_client = std::make_shared<DisClient>(rigaddr,offcb);
                }
                else
                {
                    _client = rpcframe::ClientFactory::create(rigaddr.first, rigaddr.second);

                    _client->setMessageCallBack(std::bind(&rpcframe::DisPatcher::onMessage, _dispatcher.get(), std::placeholders::_1, std::placeholders::_2));
                    _client->connect();
                }
            }
            // 相仿rpccaller接口制定了三种不同的访问的方式
            // 同步调用
            // 找到服务提供者  --  固定服务提供者

            bool call(const std::string method, const Json::Value &params, Json::Value &result)
            {
                BaseClient::Ptr client;
                if(_enable_discovery == true)
                {
                    Address_t host;
                    bool ret = _dis_client->findServer(method,host);
                    if(ret == false)
                    {
                        // method没有服务提供者
                        ELOG("%s没有服务提供者",method.c_str());
                        return false;
                    }
                    // 获取到了连接
                    client = getClient(host);
                    if(client.get() == nullptr)
                    {
                        // 构建新的连接
                        client = createNewClient(host);
                    }
                }
                else
                {
                    // 直接获取默认的连接
                    client = _client;
                }
                return _caller->call(client->connection(),method,params,result);
            }
            // 异步调用
            bool call(const std::string method, const Json::Value &params, RpcCaller::JsonAsyncReponse &result)
            {
                BaseClient::Ptr client;
                if(_enable_discovery == true)
                {
                    Address_t host;
                    bool ret = _dis_client->findServer(method,host);
                    if(ret == false)
                    {
                        // method没有服务提供者
                        ELOG("%s没有服务提供者",method.c_str());
                        return false;
                    }
                    // 获取到了连接
                    client = getClient(host);
                    if(client.get() == nullptr)
                    {
                        // 构建新的连接
                        client = createNewClient(host);
                    }
                }
                else
                {
                    // 直接获取默认的连接
                    client = _client;
                }
                return _caller->call(client->connection(),method,params,result);
            }
            // 设置回调函数
            bool call(const std::string method, const Json::Value &params, const RpcCaller::JsonResponseCallBack &callback)
            {
                BaseClient::Ptr client;
                if(_enable_discovery == true)
                {
                    Address_t host;
                    bool ret = _dis_client->findServer(method,host);
                    if(ret == false)
                    {
                        // method没有服务提供者
                        ELOG("%s没有服务提供者",method.c_str());
                        return false;
                    }
                    // 获取到了连接
                    client = getClient(host);
                    if(client.get() == nullptr)
                    {
                        // 构建新的连接
                        client = createNewClient(host);
                    }
                }
                else
                {
                    // 直接获取默认的连接
                    client = _client;
                }
                return _caller->call(client->connection(),method,params,callback);
            }

            BaseClient::Ptr createNewClient(const Address_t& addr)
            {
                // 进行服务发现
                ELOG("本地没有缓存进行服务发现");
                BaseClient::Ptr client = rpcframe::ClientFactory::create(addr.first, addr.second);

                client->setMessageCallBack(std::bind(&rpcframe::DisPatcher::onMessage, _dispatcher.get(), std::placeholders::_1, std::placeholders::_2));
                client->connect();
                // 放入连接池
                appendNewClient(addr,client);
                return client;
            }

            BaseClient::Ptr getClient(const Address_t& host)
            {
                std::unique_lock<std::mutex> lock(_mtx);
                if(_rpc_clients.count(host) == 0) return nullptr;
                else return _rpc_clients[host];
            }

            void appendNewClient(const Address_t& host,const BaseClient::Ptr& client)
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _rpc_clients.insert({host,client});
            }
            void delClient(const Address_t& host)
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _rpc_clients.erase(host);
            }
        private:
            bool _enable_discovery;
            DisClient::Ptr _dis_client; // 发现客户端
            Requestor::Ptr _requestor;  // 请求的管理
            DisPatcher::Ptr _dispatcher;
            RpcCaller::Ptr _caller;  // 构建caller请求
            BaseClient::Ptr _client; // 从client中获取connection
            std::mutex _mtx;
            // hash<method,方法组>
            // [110.14.41.69:9090 : cons]
            std::unordered_map<Address_t,BaseClient::Ptr,AddressHash> _rpc_clients; // 服务发现的客户端连接池
        };

        class TopicClient
        {
        public:
            TopicClient(const Address_t& rigaddr) : 
                _requestor(std::make_shared<Requestor>()),
                _dispatcher(std::make_shared<DisPatcher>()),
                _topic_manager(std::make_shared<TopicManager>(_requestor))
            {
                auto cb = std::bind(&rpcframe::client::Requestor::onResponse, _requestor.get(), std::placeholders::_1, std::placeholders::_2);
                _dispatcher->rigisterHandler<rpcframe::BaseMessage>(rpcframe::Mtype::RSP_TOP, cb);

                auto msg_cb = [topic_manager = _topic_manager](const BaseConnection::Ptr& con,TopicRequest::Ptr& msg)->void
                {
                    topic_manager->onPublish(con,msg);
                };
                _dispatcher->rigisterHandler<rpcframe::TopicRequest>(Mtype::REQ_TOP,msg_cb);
                _client = rpcframe::ClientFactory::create(rigaddr.first, rigaddr.second);
                _client->setMessageCallBack(std::bind(&rpcframe::DisPatcher::onMessage, _dispatcher.get(), std::placeholders::_1, std::placeholders::_2));
                _client->connect();
            }
            bool createTopic(std::string& method)
            {
                return _topic_manager->createTopic(_client->connection(),method);
            }

            bool removeTopic(std::string& method)
            {   
                return _topic_manager->removeTopic(_client->connection(),method);
            }

            bool subscribeTopic(std::string& method,const TopicManager::SubCallBack& cb)
            {
                return _topic_manager->subscribeTopic(_client->connection(),method,cb);
            }

            bool cancelTopic(std::string& method)
            {
                return _topic_manager->cancelTopic(_client->connection(),method);
            }

            bool publishTopic(std::string& method,const std::string& msg)
            {
                return _topic_manager->publishTopic(_client->connection(),method,msg);
            }   

        private:
            Requestor::Ptr _requestor;  // 请求的管理
            DisPatcher::Ptr _dispatcher;
            BaseClient::Ptr _client;
            TopicManager::Ptr _topic_manager;
        };
    }
}