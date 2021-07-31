#pragma once

#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <memory>
#include <enet/enet.h>

#include "json.hpp"
#include "all_hashes_mat.hpp"
#include "intro_msg_mat.hpp"

namespace Poco
{
    /**
     * Poco is the consensus algorithm. -- Proof-Of-Chosen-Ones
     */

    class PocoCrowd
    {
    public:
        void create_and_send_block();
        static nlohmann::json get_block_j();
        static void set_block_j(nlohmann::json block_j);
        static std::string get_hash_of_new_block();
        static void set_hash_of_new_block(std::string block);
        void reward_for_chosen_ones(std::string co_from_this_block, nlohmann::json chosen_ones_j);
    private:
        void inform_chosen_ones(std::string my_next_block, nlohmann::json block_j, std::string full_hash_req, nlohmann::json rocksdb_out);
    private:
        static nlohmann::json block_j_;
        std::shared_ptr<std::stack<std::string>> s_shptr_ = std::make_shared<std::stack<std::string>>();
        static std::string hash_of_block_;
        IntroMsgVec intro_msg_vec_;
        IpHEmail ip_hemail_vec_;
    };
}