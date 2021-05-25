#include "p2p_network_c.hpp"

using namespace Coin;

void P2pNetworkC::start_coin()
{
    // input to create a transaction (tx)
    while (true)
    {
        std::string to_full_hash = "", amount = "";
        std::cout << "Tx to: ";
        std::cin >> to_full_hash;

        if (to_full_hash == "q")
        {
            // TODO send an intro_offline request here

            P2pNetwork pn;
            pn.set_quit_server_req(true);
            break;
        }

        std::cout << "Amount: ";
        std::cin >> amount;

        std::cout << std::endl;
    }
}