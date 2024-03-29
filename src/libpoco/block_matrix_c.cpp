#include <utility>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include "configdir.hpp"
#include "protocol_c.hpp"
#include "merkle_tree_c.hpp"

#include "print_or_log.hpp"

#include "block_matrix.hpp"

using namespace Poco;

void BlockMatrixC::add_block_to_block_vector(nlohmann::json block_j)
{
    std::shared_ptr<nlohmann::json> shared_block;
    shared_block = std::make_shared<nlohmann::json> (block_j);
    block_vector_.push_back(shared_block);
}

void BlockMatrixC::add_block_vector_to_block_matrix()
{
    block_matrix_.push_back(block_vector_);

    block_vector_.clear();
}

std::vector<std::vector<std::shared_ptr<nlohmann::json>>> BlockMatrixC::get_block_matrix()
{
    return block_matrix_;
}

void BlockMatrixC::reset_block_matrix()
{
    block_matrix_.clear();
}

// void BlockMatrixC::remove_blocks_from_block_matrix()
// {
//     //
// }

void BlockMatrixC::add_received_block_to_received_block_vector(nlohmann::json block_j)
{
    std::shared_ptr<nlohmann::json> shared_block;
    shared_block = std::make_shared<nlohmann::json> (block_j);
    received_block_vector_.push_back(shared_block);
}

void BlockMatrixC::add_received_block_vector_to_received_block_matrix()
{
    received_block_matrix_.push_back(received_block_vector_);

    received_block_vector_.clear();
}

std::vector<std::vector<std::shared_ptr<nlohmann::json>>> BlockMatrixC::get_received_block_matrix()
{
    return received_block_matrix_;
}

void BlockMatrixC::sifting_function_for_both_block_matrices()
{
    // Compare block_matrix with received_block_matrix and remove not received entries from block_matrix

    Common::Print_or_log pl;
    pl.handle_print_or_log({"evaluate_both_block_matrices"});

    // pl.handle_print_or_log({"evaluate_both_block_matrices m ", std::to_string(get_block_matrix().size())});
    // pl.handle_print_or_log({"evaluate_both_block_matrices v " << std::to_string(get_block_matrix().back().size())});
    // pl.handle_print_or_log({"evaluate_both_block_matrices rm " << std::to_string(get_received_block_matrix().size())});
    // pl.handle_print_or_log({"evaluate_both_block_matrices rv " << std::to_string(get_received_block_matrix().back().size())});

    for (uint16_t i = 0; i < get_block_matrix().back().size(); i++)
    {
        bool found = false;

        std::vector<std::shared_ptr<nlohmann::json>>::iterator it;
        it = get_block_matrix().back().begin() + i;

        for (uint16_t j = 0; j < get_received_block_matrix().back().size(); j++)
        {
            if (*get_block_matrix().back().at(i) == *get_received_block_matrix().back().at(j))
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
            get_block_matrix().back().erase(it);
            continue;
        }
    }

    get_received_block_matrix().back().clear();
}

void BlockMatrixC::save_final_block_to_file()
{
    Common::Print_or_log pl;
    Crowd::Protocol proto;
    std::string latest_block = proto.get_last_block_nr();

    // if oldest vector in block matrix has size 1 --> that's the final block (= mined)
    nlohmann::json block_j;
    if (get_block_matrix().front().size() == 1)
    {
        block_j = *get_block_matrix().front().at(0);
    }
    else
    {
        return;
    }

    // hashing of the whole new block
    std::string block_s;

    // create genesis or add to blockchain
    boost::system::error_code c;
    ConfigDir cd;
    std::string blockchain_folder_path = cd.GetConfigDir() + "blockchain/crowd";

    if (!boost::filesystem::exists(blockchain_folder_path))
    {
        boost::filesystem::create_directories(blockchain_folder_path);
    }

    bool isDir = boost::filesystem::is_directory(blockchain_folder_path, c);
    bool isEmpty = boost::filesystem::is_empty(blockchain_folder_path);

    if(!isDir)
    {
        pl.handle_print_or_log({"Error Response:", std::to_string(c.value())});
    }
    else
    {
        if (latest_block != "no blockchain present in folder")
        {
            pl.handle_print_or_log({"Directory not empty"});
            block_s = block_j.dump();

            uint32_t first_chars = 11 - latest_block.length();
            std::string number = "";
            for (int i = 0; i <= first_chars; i++)
            {
                number.append("0");
            }
            number.append(latest_block);

            std::string block_file = "blockchain/crowd/block_" + number + ".json";
            if (!boost::filesystem::exists(blockchain_folder_path + "/block_" + number + ".json"))
            {
                cd.CreateFileInConfigDir(block_file, block_s); // TODO: make it count
            }
        }
        else
        {
            pl.handle_print_or_log({"Is a directory, is empty"});

            Crowd::merkle_tree mt;
            block_j["prev_hash"] = mt.get_genesis_prev_hash();
            block_s = block_j.dump();
            std::string block_file = "blockchain/crowd/block_000000000000.json";
            if (!boost::filesystem::exists(blockchain_folder_path + "/block_000000000000.json"))
            {
                cd.CreateFileInConfigDir(block_file, block_s);
            }
        }
    }

    return;
}

std::vector<std::shared_ptr<nlohmann::json>> BlockMatrixC::block_vector_;
std::vector<std::vector<std::shared_ptr<nlohmann::json>>> BlockMatrixC::block_matrix_ = {};
std::vector<std::shared_ptr<nlohmann::json>> BlockMatrixC::received_block_vector_;
std::vector<std::vector<std::shared_ptr<nlohmann::json>>> BlockMatrixC::received_block_matrix_ = {};
