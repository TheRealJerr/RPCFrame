#include "../../server/RpcServer.hpp"

int main()
{
    auto server = std::make_shared<rpcframe::server::TopicServer>(7070);
    server->start();
    return 0;
}