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
        static uint64_t calculate_dev_payment_numbers(uint64_t amount);
        static void set_new_rewards(bool new_rewards);
        static bool get_new_rewards();
    private:
        static std::string calculate_dev_payment(std::string amount);
    private:
        static std::vector<std::pair<std::string, std::shared_ptr<std::vector<std::string>>>> transactions_;
        static std::map<std::string, std::vector<std::string>> latest_transaction_;
        static bool new_rewards_;
    };
}