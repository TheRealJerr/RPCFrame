#include "../../common/detail.hpp"
#include "../../server/rpc_server.hpp"

int main() 
{
    bitrpc::server::RegistryServer reg_server(8080);
    reg_server.start();
    return 0;
}