#pragma once
#include "../Common/message.hpp"
#include "../Common/net.hpp"
#include "../Common/dispatcher.hpp"
#include "RpcRouter.hpp"
#include "RpcRigister.hpp"
#include "RpcTopic.hpp"
#include "../client/RpcClient.hpp"
#include <atomic>
namespace rpcframe
{
    namespace server
    {
        // 这是一个注册服务端
        // 他的作用的管理RPC服务端的上下线,以及提供根据descoverier提供对应的host
        class RigistryServer
        {
        public:
            using Ptr = std::shared_ptr<RigistryServer>;
            RigistryServer(int port):
                _dispatcher(std::make_shared<DisPatcher>()),
                _pd_manager(std::make_shared<PDManager>())
            {
                auto cb = [pd_manager = _pd_manager](const BaseConnection::Ptr &con, ServiceRequest::Ptr &msg)->void
                {
                    pd_manager->onServiceRequest(con,msg);
                };
                _dispatcher->rigisterHandler<ServiceRequest>(Mtype::REQ_SERVICE,cb);
                auto sv_cb = [dispatcher = _dispatcher](const BaseConnection::Ptr& con,BaseMessage::Ptr& msg)
                {
                    dispatcher->onMessage(con,msg);
                };
                auto cls_cb = [pd_manager = _pd_manager](const BaseConnection::Ptr& con)
                {
                    pd_manager->onConnectionShutDown(con);
                };
                _server = ServerFactory::create(port);
                _server->setMessageCallBack(sv_cb);
                _server->setCloseCallBack(cls_cb);
                
            }
            void start()
            {
                _server->start();
            }
        private:
            
        private:
            BaseServer::Ptr _server; // 服务端
            DisPatcher::Ptr _dispatcher;// 任务派发器
            PDManager::Ptr _pd_manager; // provider和descoverier的管理者
        };

        // 定义一个Rpc的服务器
        class RpcServer
        {
        public:
            using Ptr = std::shared_ptr<RpcServer>;
            // rpcserver端有两套地址信息,第一套是提供端的地址信息 ：rpc服务器的对外访问地址
            // 第二套是监听的地址
            // 对于云服务器， 监听的是局域网地址,但是访问的是公网的地址
            // * @access_addr : 提供给客户端访问的ip地址
            // * @rig_addr : 注册中的ip地址和端口
            // 
            RpcServer(const Address_t& access_addr,bool enable_rig = false,const Address_t& rig_addr = Address_t()):
                _enable_rigistry(enable_rig),
                _rig_addr(rig_addr),_access_addr(access_addr),
                _dispatcher(std::make_shared<DisPatcher>()),
                _router(std::make_shared<RpcRouter>())               
            {
                // 是否需要进行服务注册
                if(_enable_rigistry)
                {
                    // 构造注册中心客户端
                    _rig_client = std::make_shared<client::RigClient>(_rig_addr);
                }
                // rpc Server 我们这个服务器值解决REQ_RPC的请求(
                auto rpc_handler = [router = _router](const BaseConnection::Ptr& con,RpcRequest::Ptr& msg)
                {
                    return router->onRpcRequest(con,msg);
                };
                _dispatcher->rigisterHandler<RpcRequest>(Mtype::REQ_RPC,rpc_handler);
                _server = ServerFactory::create(access_addr.second);
                auto msg_cb = [dispatcher = _dispatcher](const BaseConnection::Ptr& con,BaseMessage::Ptr& msg)
                {
                    return dispatcher->onMessage(con,msg);
                };
                _server->setMessageCallBack(msg_cb);
            }
            void rigisterMethod(const ServiceDescribe::Ptr& service)
            {
                _router->rigisterMethod(service);
                if(_enable_rigistry)
                {
                    _rig_client->rigisteMethod(service->Method(),_access_addr);
                }

            }

            void start()
            {
                _server->start();
            }
        private:
            Address_t _rig_addr;
            Address_t _access_addr;
            RpcRouter::Ptr _router;
            DisPatcher::Ptr _dispatcher;
            BaseServer::Ptr _server;
            // 是否启用注册功能
            bool _enable_rigistry;
            // 注册的客户端
            rpcframe::client::RigClient::Ptr _rig_client;
        };
        class TopicServer
        {
        public:
            using Ptr = std::shared_ptr<TopicServer>;
            TopicServer(int port):
                _dispatcher(std::make_shared<DisPatcher>()),
                _topic_manager(std::make_shared<TopicManager>())
            {
                auto cb = [topic_manager = _topic_manager](const BaseConnection::Ptr &con, TopicRequest::Ptr &msg)->void
                {
                    topic_manager->onTopicMessage(con,msg);
                };
                _dispatcher->rigisterHandler<TopicRequest>(Mtype::REQ_TOP,cb);
                auto sv_cb = [dispatcher = _dispatcher](const BaseConnection::Ptr& con,BaseMessage::Ptr& msg)
                {
                    dispatcher->onMessage(con,msg);
                };
                auto cls_cb = [topic_manager = _topic_manager](const BaseConnection::Ptr& con)
                {
                    topic_manager->onShutDown(con);
                };
                _server = ServerFactory::create(port);
                _server->setMessageCallBack(sv_cb);
                _server->setCloseCallBack(cls_cb);
                
            }
            void start()
            {
                _server->start();
            }
        private:
            
        private:
            BaseServer::Ptr _server; // 服务端
            DisPatcher::Ptr _dispatcher;// 任务派发器
            TopicManager::Ptr _topic_manager; // provider和descoverier的管理者
            
        };

    }
}


