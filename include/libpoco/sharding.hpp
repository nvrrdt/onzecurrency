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
        
    private:
        std::map<std::string, uint256_t> fair_partitioning();
        std::pair<std::string, uint256_t> get_fair_user_id(std::string user_id);
        uint32_t get_amount_of_shards();
        uint256_t get_shard_distance();
    };

    class NetworkSharding /* TODO */
    {
    public:
        
    private:
        
    };
}