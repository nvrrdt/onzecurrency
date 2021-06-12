#pragma once

#include <iostream>
#include <string>

#include <stack>
#include <memory>

#include "json.hpp"

#include "merkle_tree.hpp"

using namespace Crowd;

namespace Coin
{
    class merkle_tree_c: merkle_tree
    {
    public:
        // int prep_block_creation();
        nlohmann::json create_block_c(std::string &datetime, std::string &root_hash_data, nlohmann::json &txs_data_j);
        std::string time_now_c();
        std::shared_ptr<std::stack<std::string>> calculate_root_hash_c(std::shared_ptr<std::stack<std::string>> &s_shptr);
        std::string save_block_to_file_c(nlohmann::json &block_j, std::string &latest_block);
        // void set_genesis_prev_hash();
        // std::string get_genesis_prev_hash();
    private:
        std::shared_ptr<std::stack<std::string>> pop_two_and_hash_c(std::shared_ptr<std::stack<std::string>> &s_shptr);
        // std::string genesis_prev_hash_;
    };
}
