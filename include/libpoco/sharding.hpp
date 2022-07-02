#pragma once

#include <iostream>
#include <string>
#include <map>

#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision;

#include "print_or_log.hpp"

namespace Poco
{
    class DatabaseSharding
    {
    public:
        std::vector<std::string> get_shard_users(std::string user_id);
        std::pair<std::uint32_t, std::pair<uint256_t, uint256_t>> get_fair_shard_range(std::string user_id);
        uint32_t which_shard_to_process();
        uint32_t get_amount_of_shards();
    private:
        std::map<std::string, uint256_t> fair_partitioning();
        std::pair<std::string, uint256_t> get_fair_user_id(std::string user_id);
    };

    class NetworkSharding /* TODO */
    {
    public:
        
    private:
        
    };
}