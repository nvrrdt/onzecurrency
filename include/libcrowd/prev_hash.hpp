#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace Crowd
{
    class PrevHash
    {
    public:
        void save_my_prev_hash_to_file(std::string &prev_hash);
        std::string get_my_prev_hash_from_file();
        std::string calculate_hash_from_last_block();
        // std::string get_prev_hash_from_the_last_block();
        std::vector<std::string> get_prev_hashes_vec_from_files();
        std::vector<std::string> get_blocks_vec_from_files();
    private:
    };
}