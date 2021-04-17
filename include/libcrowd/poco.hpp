#pragma once

#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <memory>
#include <enet/enet.h>

#include "json.hpp"

namespace Crowd
{
    /**
     * Poco is the consensus algorithm. -- Proof-Of-Chosen-Ones
     */

    class Poco
    {
    public:
        Poco(std::vector<nlohmann::json> &message_j_vec, std::map<enet_uint32, std::string> &all_full_hashes);
        nlohmann::json get_block_j();
        std::string get_hash_of_new_block();
    private:
        void inform_chosen_ones(std::string my_next_block, nlohmann::json block_j, std::map<enet_uint32, std::string> &all_full_hashes);
    private:
        void set_hash_of_new_block(std::string block);
        std::vector<nlohmann::json> message_j_vec_;
        nlohmann::json block_j_;
        std::shared_ptr<std::stack<std::string>> s_shptr_ = std::make_shared<std::stack<std::string>>();
        std::string hash_of_block_;
    };
}