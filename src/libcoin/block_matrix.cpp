#include <utility>

#include "block_matrix.hpp"

using namespace Coin;

void BlockMatrix::add_block_to_block_vector(nlohmann::json block_j)
{
    std::shared_ptr<nlohmann::json> shared_block;
    shared_block = std::make_shared<nlohmann::json> (block_j);
    block_vector_.push_back(shared_block);
}

void BlockMatrix::add_block_vector_to_block_matrix()
{
    block_matrix_.push_back(block_vector_);

    block_vector_.clear();
}

std::vector<std::vector<std::shared_ptr<nlohmann::json>>> BlockMatrix::get_block_matrix()
{
    return block_matrix_;
}

void BlockMatrix::reset_block_matrix()
{
    block_matrix_.clear();
}

// void BlockMatrix::remove_blocks_from_block_matrix()
// {
//     //
// }

std::vector<std::shared_ptr<nlohmann::json>> BlockMatrix::block_vector_;
std::vector<std::vector<std::shared_ptr<nlohmann::json>>> BlockMatrix::block_matrix_ = {};
