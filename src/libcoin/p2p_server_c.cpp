// The plan to create coin:
// - wished call for a payment: onze1blahblah 1.01 ((to peer) (value in onze)
// - 1. create rudimentary input for this call
// - 2. intro_tx and new_tx, the chose_ones verify the funds of the payer
// - 3. intro_block_c and new_block_c, the chosen_ones are rewarded an onze
// - headless state handling

#include "p2p_network_c.hpp"

using namespace Coin;

void P2pNetworkC::handle_read_server_c(nlohmann::json buf_j)
{
    //
    std::cout << "buf_j server " << buf_j << std::endl;

    std::string req = buf_j["req"];
    std::map<std::string, int> req_conversion;
    req_conversion["intro_tx"] =            20;
    req_conversion["new_tx"] =              21;

    switch (req_conversion[req])
    {
        case 20:    intro_tx(buf_j);
                    break;
        case 21:    new_tx(buf_j);
                    break;
    }
}

void P2pNetworkC::intro_tx(nlohmann::json buf_j)
{
    //
}

void P2pNetworkC::new_tx(nlohmann::json buf_j)
{
    //
}