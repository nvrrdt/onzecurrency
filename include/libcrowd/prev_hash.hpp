#pragma once

#include <iostream>
#include <string>

#include "random.hpp"

namespace Crowd
{
    class PrevHash
    {
    public:
        void save_prev_hash_to_file(std::string &prev_hash);
        std::string get_prev_hash_from_file();
    private:
    };
}