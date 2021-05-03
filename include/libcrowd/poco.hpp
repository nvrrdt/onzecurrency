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
        static nlohmann::json get_block_j();
        static void set_block_j(nlohmann::json block_j);
        static std::string get_hash_of_new_block();
        static void set_hash_of_new_block(std::string block);
    private:
        void inform_chosen_ones(std::string my_next_block, nlohmann::json block_j, std::string full_hash_req);
    private:
        static nlohmann::json block_j_;
        std::shared_ptr<std::stack<std::string>> s_shptr_ = std::make_shared<std::stack<std::string>>();
        static std::string hash_of_block_;
        MessageVec message_j_vec_; // maybe std::shared_ptr<MessageVec> message_j_vec_ = std::make_shared<MessageVec>() ?? compare with header!!
        AllFullHashes all_full_hashes_;
    };
}