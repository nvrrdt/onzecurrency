#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace Crowd
{
    class FullHash
    {
    public:
        void save_full_hash_to_file(std::string &full_hash);
        std::string get_full_hash_from_file();
    private:
    };
}