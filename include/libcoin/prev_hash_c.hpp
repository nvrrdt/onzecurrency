#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "prev_hash.hpp"

using namespace Crowd;

namespace Coin
{
    class PrevHashC: PrevHash
    {
    public:
        std::string calculate_hash_from_last_block_c();
        std::vector<std::string> get_prev_hashes_vec_from_files_c();
        std::vector<std::string> get_blocks_vec_from_files_c();
    private:
    };
}