#include <utility>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include "configdir.hpp"
#include "p2p.hpp"
#include "merkle_tree.hpp"

#include "block_matrix.hpp"

using namespace Poco;

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

void BlockMatrix::sifting_function_for_both_block_matrices()
{
    // Compare block_matrix with received_block_matrix and remove not received entries from block_matrix

    std::cout << "sifting_function_for_both_block_matrices" << std::endl;

    // std::cout << "evaluate_both_block_matrices m " << get_block_matrix().size() << std::endl;
    // std::cout << "evaluate_both_block_matrices v " << get_block_matrix().back().size() << std::endl;
    // std::cout << "evaluate_both_block_matrices rm " << get_received_block_matrix().size() << std::endl;
    // std::cout << "evaluate_both_block_matrices rv " << get_received_block_matrix().back().size() << std::endl;

    if (!get_block_matrix().empty() && get_block_matrix().back().size() == 100) // TODO this 100 is a variable that can be changed, there are others as well
    {
        // get prev_hashes from within the latest vector and compare with the hashes from before latest vector
        Crowd::PrevHash ph;
        for (uint16_t i = 0; i < ph.get_prev_hashes_from_block_matrix_contents().size(); i++)
        {
            bool found = false;

            std::vector<std::shared_ptr<nlohmann::json>>::iterator it;
            it = get_block_matrix().at(i).begin();
std::cout << "1112" << std::endl;
            std::vector<std::shared_ptr<std::string>>::iterator itt;
            itt = ph.get_prev_hashes_from_block_matrix_contents().at(i).begin();
std::cout << "1113" << std::endl;
            std::vector<std::shared_ptr<std::string>>::iterator ittt;
            ittt = ph.calculate_hashes_from_block_matrix().at(i).begin();
std::cout << "1114" << std::endl;
            for (uint16_t j = 0; j < ph.get_prev_hashes_from_block_matrix_contents().at(i).size(); j++)
            {
                if (j == 0) continue;

                if (*ph.get_prev_hashes_from_block_matrix_contents().at(i).at(j) == *ph.calculate_hashes_from_block_matrix().at(i).at(j-1))
                {
                    found = true;
                    break;
                }
            }
std::cout << "1115" << std::endl;
            if (found)
            {
                found == false;
                continue;
            }
            else
            {
                get_block_matrix().at(i).erase(it);
                ph.get_prev_hashes_from_block_matrix_contents().at(i).erase(itt);
                ph.calculate_hashes_from_block_matrix().at(i).erase(ittt);
                continue;
            }
std::cout << "1116" << std::endl;
        }
    }

    if (!get_block_matrix().empty() && !get_received_block_matrix().empty())
    {
        // schrink block_matrix so only received_blocks remain
        for (uint16_t i = 0; i < get_block_matrix().back().size(); i++)
        {
            bool found = false;

            std::vector<std::shared_ptr<nlohmann::json>>::iterator it;
            it = get_block_matrix().back().begin();
std::cout << "1117" << std::endl;
            for (uint16_t j = 0; j < get_received_block_matrix().back().size(); j++)
            {
                if (*get_block_matrix().back().at(i) == *get_received_block_matrix().back().at(j))
                {
                    found = true;
                    break;
                }
            }
std::cout << "1118" << std::endl;
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
std::cout << "1119" << std::endl;
        get_received_block_matrix().back().clear();
    }
}

void BlockMatrix::save_final_block_to_file()
{
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
    Crowd::ConfigDir cd;
    std::string blockchain_folder_path = cd.GetConfigDir() + "blockchain/crowd";

    if (!boost::filesystem::exists(blockchain_folder_path))
    {
        boost::filesystem::create_directories(blockchain_folder_path);
    }

    bool isDir = boost::filesystem::is_directory(blockchain_folder_path, c);
    bool isEmpty = boost::filesystem::is_empty(blockchain_folder_path);

    if(!isDir)
    {
        std::cout << "Error Response: " << c << std::endl;
    }
    else
    {
        if (latest_block != "no blockchain present in folder")
        {
            std::cout << "Directory not empty" << std::endl;
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
            std::cout << "Is a directory, is empty" << std::endl;

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

std::vector<std::shared_ptr<nlohmann::json>> BlockMatrix::block_vector_;
std::vector<std::vector<std::shared_ptr<nlohmann::json>>> BlockMatrix::block_matrix_ = {};
std::vector<std::shared_ptr<nlohmann::json>> BlockMatrix::received_block_vector_;
std::vector<std::vector<std::shared_ptr<nlohmann::json>>> BlockMatrix::received_block_matrix_ = {};
