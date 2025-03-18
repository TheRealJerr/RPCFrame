#include "../../common/detail.hpp"
#include "../../client/rpc_client.hpp"

void callback(const Json::Value &result) {
    ILOG("callback result: %d", result.asInt());
}

int main()
{
    bitrpc::client::RpcClient client(false, "127.0.0.1", 9090);

    Json::Value params, result;
    params["num1"] = 11;
    params["num2"] = 22;
    bool ret = client.call("Add", params, result);
    if (ret != false) {
        ILOG("result: %d", result.asInt());
    }

    bitrpc::client::RpcCaller::JsonAsyncResponse res_future;
    params["num1"] = 33;
    params["num2"] = 44;
    ret = client.call("Add", params, res_future);
    if (ret != false) {
        result = res_future.get();
        ILOG("result: %d", result.asInt());
    }

    params["num1"] = 55;
    params["num2"] = 66;
    ret = client.call("Add", params, callback);
    DLOG("-------\n");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}