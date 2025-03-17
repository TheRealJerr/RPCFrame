#include "../../common/detail.hpp"
#include "../../server/rpc_server.hpp"

void Add(const Json::Value &req, Json::Value &rsp) {
    int num1 = req["num1"].asInt();
    int num2 = req["num2"].asInt();
    rsp = num1 + num2;
}
int main()
{
    std::unique_ptr<bitrpc::server::SDescribeFactory> desc_factory(new bitrpc::server::SDescribeFactory());
    desc_factory->setMethodName("Add");
    desc_factory->setParamsDesc("num1", bitrpc::server::VType::INTEGRAL);
    desc_factory->setParamsDesc("num2", bitrpc::server::VType::INTEGRAL);
    desc_factory->setReturnType(bitrpc::server::VType::INTEGRAL);
    desc_factory->setCallback(Add);

    bitrpc::server::RpcServer server(bitrpc::Address("127.0.0.1", 9090), true, bitrpc::Address("127.0.0.1", 8080));
    server.registerMethod(desc_factory->build());
    server.start();
    return 0;
}