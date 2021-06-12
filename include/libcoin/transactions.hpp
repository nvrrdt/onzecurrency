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
        static std::vector<std::pair<std::string, std::shared_ptr<std::vector<std::string>>>> get_transactions();
        static void reset_transactions();
        static void remove_tx_from_transactions(std::string tx_hash);
        static void set_latest_transaction(std::string tx_hash, std::vector<std::string> vec);
        static std::map<std::string, std::vector<std::string>> get_latest_transaction();
    private:
        static std::vector<std::pair<std::string, std::shared_ptr<std::vector<std::string>>>> transactions_;
        static std::map<std::string, std::vector<std::string>> latest_transaction_;
    };
}