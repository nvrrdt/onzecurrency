#pragma once

#include <iostream>
#include <string>

#include <stack>
#include <memory>

#include "json.hpp"

namespace Crowd
{
    class merkle_tree
    {
    public:
        int prep_block_creation();
        nlohmann::json create_block(std::string &datetime, std::string &root_hash_data, nlohmann::json &entry_data_j, nlohmann::json &exit_data_j);
        std::string time_now();
        std::shared_ptr<std::stack<std::string>> calculate_root_hash(std::shared_ptr<std::stack<std::string>> &s_shptr);
        std::string save_block_to_file(nlohmann::json &block_j, std::string &latest_block);
        std::string get_genesis_prev_hash();
    private:
        std::shared_ptr<std::stack<std::string>> pop_two_and_hash(std::shared_ptr<std::stack<std::string>> &s_shptr);
    };
}
