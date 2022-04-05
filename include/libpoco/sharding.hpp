#pragma once

#include <iostream>
#include <string>
#include <map>

#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision;

namespace Poco
{
    class DatabaseSharding
    {
    public:
        
    private:
        std::map<std::string, uint256_t> fair_partitioning();
    };

    class NetworkSharding /* TODO */
    {
    public:
        
    private:
        
    };
}