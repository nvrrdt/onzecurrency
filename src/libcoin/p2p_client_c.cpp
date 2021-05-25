#include "p2p_network_c.hpp"

using namespace Coin;

void P2pNetworkC::handle_read_client_c(nlohmann::json buf_j)
{
    //
    std::cout << "buf_j client " << buf_j << std::endl;
}