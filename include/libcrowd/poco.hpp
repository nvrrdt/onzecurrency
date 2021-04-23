#pragma once

#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <memory>
#include <enet/enet.h>

#include "json.hpp"
#include "all_full_hashes.hpp"
#include "message_vec.hpp"

namespace Crowd
{
    /**
     * Poco is the consensus algorithm. -- Proof-Of-Chosen-Ones
     */

    class Poco
    {
    public:
        void create_and_send_block();
        nlohmann::json get_block_j();
        static std::string get_hash_of_new_block();
    private:
        void inform_chosen_ones(std::string my_next_block, nlohmann::json block_j);
    private:
        static void set_hash_of_new_block(std::string block);
        nlohmann::json block_j_;
        std::shared_ptr<std::stack<std::string>> s_shptr_ = std::make_shared<std::stack<std::string>>();
        static std::string hash_of_block_;
        MessageVec message_j_vec_;
        AllFullHashes all_full_hashes_;
    };
}