#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <memory>

namespace Coin
{
    class Transactions
    {
    public:
        static void add_tx_to_transactions(std::string full_hash_req, std::string to_full_hash, std::string amount);
        static std::map<std::string, std::shared_ptr<std::vector<std::string>>> get_transactions();
        static void reset_transactions();
    private:
        static std::map<std::string, std::shared_ptr<std::vector<std::string>>> transactions_;
    };
}