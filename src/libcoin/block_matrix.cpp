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

void BlockMatrix::add_received_block_to_received_block_vector(nlohmann::json block_j)
{
    std::shared_ptr<nlohmann::json> shared_block;
    shared_block = std::make_shared<nlohmann::json> (block_j);
    received_block_vector_.push_back(shared_block);
}

void BlockMatrix::add_received_block_vector_to_received_block_matrix()
{
    received_block_matrix_.push_back(received_block_vector_);

    received_block_vector_.clear();
}

std::vector<std::vector<std::shared_ptr<nlohmann::json>>> BlockMatrix::get_received_block_matrix()
{
    return received_block_matrix_;
}

void BlockMatrix::evaluate_both_block_matrices()
{
    // Compare block_matrix with received_block_matrix and remove not received entries from block_matrix

    for (uint16_t i = 0; i < get_block_matrix().back().size(); i++)
    {
        bool found = false;

        for (uint16_t j = 0; j < get_received_block_matrix().back().size(); j++)
        {
            if (get_block_matrix().back().at(i) == get_received_block_matrix().back().at(j))
            {
                found = true;
                break;
            }
        }

        if (found)
        {
            found == false;
            continue;
        }
        else
        {
            get_block_matrix().back().erase(get_block_matrix().back().begin() + i);
            continue;
        }
    }

    get_received_block_matrix().back().clear();
}

std::vector<std::shared_ptr<nlohmann::json>> BlockMatrix::block_vector_;
std::vector<std::vector<std::shared_ptr<nlohmann::json>>> BlockMatrix::block_matrix_ = {};
std::vector<std::shared_ptr<nlohmann::json>> BlockMatrix::received_block_vector_;
std::vector<std::vector<std::shared_ptr<nlohmann::json>>> BlockMatrix::received_block_matrix_ = {};
