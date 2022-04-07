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
        std::pair<std::string, uint256_t> get_fair_order_nr(std::string user_id);
        void dynamic_sharding();
    };

    class NetworkSharding /* TODO */
    {
    public:
        
    private:
        
    };
}