// 表示我们的注册中心

#include "../../server/RpcServer.hpp"
#include "../../Common/detail.hpp"

int main()
{
    auto rigistry = std::make_shared<rpcframe::server::RigistryServer>(9090);
    rigistry->start();
    return 0;
}

