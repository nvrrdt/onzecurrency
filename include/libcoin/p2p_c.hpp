#pragma once

#include <string>
#include <vector>

#include "json.hpp"
#include "p2p_network.hpp"

namespace Coin
{
    class P2pC
    {
    public:
        void start_coin();

        static void set_coin_update_complete(bool completed)
        {
            coin_update_complete_ = completed;
        }
    private:
        static bool get_coin_update_complete()
        {
            return coin_update_complete_;
        }
    private:
        bool validate_full_hash(std::string to_full_hash);
        bool validate_amount(std::string amount);
        static bool coin_update_complete_;
    };
}