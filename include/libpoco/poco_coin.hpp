#pragma once

#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <memory>
#include <enet/enet.h>

// #include "poco_crowd.hpp"
#include "json.hpp"

#include "transactions.hpp"

namespace Poco
{
    class PocoCoin // : PocoCrowd
    {
    public:
        ~PocoCoin()
        {
            delete tx_;
        }
        void create_prel_blocks_c();
        void evaluate_transactions();
    private:
        void inform_chosen_ones_c(std::string my_next_block_nr, nlohmann::json block_j, std::string full_hash_req);
        void candidate_blocks_creation();
    private:
        std::shared_ptr<std::stack<std::string>> s_shptr_c_ = std::make_shared<std::stack<std::string>>();
        Transactions *tx_ = new Transactions();
        nlohmann::json block_j_c_;
    };
}