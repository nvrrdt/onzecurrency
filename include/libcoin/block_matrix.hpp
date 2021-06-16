#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "json.hpp"

namespace Coin
{
    class BlockMatrix
    {
    public:
        static void add_block_to_block_vector(nlohmann::json block_j);
        static void add_block_vector_to_block_matrix();
        static std::vector<std::vector<std::shared_ptr<nlohmann::json>>> get_block_matrix();
        static void reset_block_matrix();
        static void remove_blocks_from_block_matrix();
        static void add_received_block_to_received_block_vector(nlohmann::json block_j);
        static void add_received_block_vector_to_received_block_matrix();
        static std::vector<std::vector<std::shared_ptr<nlohmann::json>>> get_received_block_matrix();
    private:
        static std::vector<std::shared_ptr<nlohmann::json>> block_vector_;
        static std::vector<std::vector<std::shared_ptr<nlohmann::json>>> block_matrix_;
        static std::vector<std::shared_ptr<nlohmann::json>> received_block_vector_;
        static std::vector<std::vector<std::shared_ptr<nlohmann::json>>> received_block_matrix_;
    };
}