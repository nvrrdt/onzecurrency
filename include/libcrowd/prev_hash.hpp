#pragma once

#include <iostream>
#include <string>

namespace Crowd
{
    class PrevHash
    {
    public:
        void save_my_prev_hash_to_file(std::string &prev_hash);
        std::string get_my_prev_hash_from_file();
        std::string get_last_prev_hash_from_blocks();
    private:
    };
}