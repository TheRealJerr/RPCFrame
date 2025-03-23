#include "../../server/rpc_server.hpp"

int main()
{
    auto server = std::make_shared<bitrpc::server::TopicServer>(7070);
    server->start();
    return 0;
}