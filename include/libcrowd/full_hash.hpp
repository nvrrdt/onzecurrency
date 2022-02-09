#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace Crowd
{
    class FullHash
    {
    public:
        static void save_full_hash(std::string &full_hash);
        static std::string get_full_hash();
    private:
        static std::string full_hash_;
    };
}