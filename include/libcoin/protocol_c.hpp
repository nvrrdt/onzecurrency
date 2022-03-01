#pragma once

#include <string>
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>

#include <boost/array.hpp>

#include "rocksy.hpp"
#include "json.hpp"
#include "prev_hash.hpp"
#include "crypto.hpp"
#include "merkle_tree.hpp"
#include "full_hash.hpp"
#include "auth.hpp"

#include "p2p.hpp"

#include <stack>
#include <memory>

#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision;

using namespace std::chrono_literals;

using namespace std; 
using namespace boost::asio; 
using namespace boost::asio::ip;

using namespace Crowd;

namespace Coin
{
    class ProtocolC: Protocol
    {
    public:
        std::string get_last_block_nr_c();
        // std::map<int, std::string> partition_in_buckets(std::string &my_hash, std::string &next_hash);
        // std::map<uint32_t, uint256_t>layers_management(uint256_t &amount_of_peers);
        nlohmann::json get_blocks_from_c(std::string &latest_block_peer);
        std::string get_all_users_from_c(std::string &latest_block_peer);
        // std::string block_plus_one(std::string &block_nr);
    private:
        // int verify_latest_block(std::string &latest_block_peer);
        // int communicate_to_all(boost::array<char, 128> &msg);
        // std::map<int, std::string> get_calculated_hashes(std::string &my_hash, std::map<uint32_t, uint256_t> &chosen_ones_counter);
    };
}
