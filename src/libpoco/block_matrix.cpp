#include <utility>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <algorithm>

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

void BlockMatrix::add_calculated_hash_to_calculated_hash_vector(nlohmann::json block_j)
{
    Common::Crypto crypto;
    std::string str = block_j.dump();
    std::string ph = crypto.bech32_encode_sha256(str);
    std::shared_ptr<std::string> shared_ph = std::make_shared<std::string> (ph);
    calculated_hash_vector_.push_back(shared_ph);
}

void BlockMatrix::add_calculated_hash_vector_to_calculated_hash_matrix()
{
    calculated_hash_matrix_.push_back(calculated_hash_vector_);

    calculated_hash_vector_.clear();
}

std::vector<std::vector<std::shared_ptr<std::string>>> BlockMatrix::get_calculated_hash_matrix()
{
    return calculated_hash_matrix_;
}

void BlockMatrix::add_prev_hash_to_prev_hash_vector(nlohmann::json block_j)
{
    std::string prev_hash = block_j["prev_hash"];
    std::shared_ptr<std::string> shared_ph = std::make_shared<std::string> (prev_hash);
    prev_hash_vector_.push_back(shared_ph);
}

void BlockMatrix::add_prev_hash_vector_to_prev_hash_matrix()
{
    prev_hash_matrix_.push_back(prev_hash_vector_);

    prev_hash_vector_.clear();
}

std::vector<std::vector<std::shared_ptr<std::string>>> BlockMatrix::get_prev_hash_matrix()
{
    return prev_hash_matrix_;
}

void BlockMatrix::replace_block_matrix(std::vector<std::vector<std::shared_ptr<nlohmann::json>>> block_matrix)
{
    block_matrix_.clear();
    block_matrix_ = block_matrix;
}

void BlockMatrix::replace_calculated_hashes(std::vector<std::vector<std::shared_ptr<std::string>>> calculated_hashes)
{
    calculated_hash_matrix_.clear();
    calculated_hash_matrix_ = calculated_hashes;
}

void BlockMatrix::replace_prev_hashes(std::vector<std::vector<std::shared_ptr<std::string>>> hashes_from_contents)
{
    prev_hash_matrix_.clear();
    prev_hash_matrix_ = hashes_from_contents;
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

void BlockMatrix::clear_received_block_matrix()
{
    received_block_matrix_.clear();
}

void BlockMatrix::sifting_function_for_both_block_matrices()
{
    // Compare block_matrix with received_block_matrix and remove not received entries from block_matrix

    std::cout << "sifting_function_for_both_block_matrices" << std::endl;

    auto block_matrix = get_block_matrix();
    auto calculated_hashes = get_calculated_hash_matrix();
    auto hashes_from_contents = get_prev_hash_matrix();
    auto received_block_matrix = get_received_block_matrix();

    // fill with positions who get removed later
    std::vector<std::vector<int16_t>> pos = {};
    std::vector<int16_t> p = {};
    for (int16_t k = 0; k < block_matrix.size(); k++)
    {
        for (int16_t m = 0; m < block_matrix.at(k).size(); m++)
        {
            p.push_back(m);
        }
        pos.push_back(p);
        p.clear();
    }
    int16_t pos_length = pos.back().size();

    if (!block_matrix.empty() && !received_block_matrix.empty())
    {
        // remove non-received blocks in the last block_vector of block_matrix
        for (uint16_t i = block_matrix.back().size() - 1; i >= 0; i--)
        {
            if (received_block_matrix.back().empty()) break;

            for (uint16_t j = received_block_matrix.back().size() - 1; j >= 0; j--)
            {
                if (*block_matrix.back().at(i) == *received_block_matrix.back().at(j))
                {
                    std::cout << "received block found" << std::endl;

                    pos.back().erase(pos.back().begin() + i);
                    break;
                }
                else
                {
                    std::cout << "received block not found" << std::endl;
                }
            }
        }

        if (pos_length != pos.back().size())
        {
            for (int16_t n = pos.back().size() - 1; n >= 0 ; n--)
            {
                std::cout << "erase p received: " << pos.back().at(n) << std::endl;
                
                block_matrix.back().erase(block_matrix.back().begin() + pos.back().at(n));
                calculated_hashes.back().erase(calculated_hashes.back().begin() + pos.back().at(n));
                hashes_from_contents.back().erase(hashes_from_contents.back().begin() + pos.back().at(n));
            }
        }

        clear_received_block_matrix();
    }

    /**
     * The prev_hash in i+1 must correspond to the calculated hash of block in i
     * Below sifts through this:
     * 
     * last vector block_matrix:
     * create list of all prev_hashes in those files,
     * last - 1 vector block_matrix:
     * create list of calculated prev_hashes,
     * compare two lists and delete in last - 1 the blocks that aren't in last,
     * then compare last - 2 and last -1, ...,
     * just until the final block remains
     */

    if (!block_matrix.empty())
    {
        // get prev_hashes from within the latest vector and compare with the hashes from before latest vector

        // std::cout << "evaluate_both_block_matrices m " << get_block_matrix().size() << std::endl;
        // std::cout << "evaluate_both_block_matrices v " << get_block_matrix().back().size() << std::endl;
        // std::cout << "evaluate_both_block_matrices mcph " << get_calculated_hash_matrix().size() << std::endl;
        // std::cout << "evaluate_both_block_matrices vcph " << get_calculated_hash_matrix().back().size() << std::endl;
        // std::cout << "evaluate_both_block_matrices mphfc " << get_prev_hash_matrix().size() << std::endl;
        // std::cout << "evaluate_both_block_matrices vphfc " << get_prev_hash_matrix().back().size() << std::endl;

        // std::cout << "evaluate_both_block_matrices rm " << get_received_block_matrix().size() << std::endl;
        // std::cout << "evaluate_both_block_matrices rv " << get_received_block_matrix().back().size() << std::endl;

        // // for debugging purposes:
        // for (int x = 0; x < calculated_hashes.size(); x++)
        // {
        //     for (int y = 0; y < calculated_hashes.at(x).size(); y++)
        //     {
        //         std::string content = *calculated_hashes.at(x).at(y);
        //         std::cout << "calculated_hashes " << x << " " << y << " (oldest first) " << content << std::endl << std::endl;
        //     }
        // }

        // // for debugging purposes:
        // for (int v = 0; v < hashes_from_contents.size(); v++)
        // {
        //     for (int w = 0; w < hashes_from_contents.at(v).size(); w++)
        //     {
        //         std::string content = *hashes_from_contents.at(v).at(w);
        //         std::cout << "hashes_from_contents " << v << " " << w << " (oldest first) " << content << std::endl << std::endl;
        //     }
        // }

        for (int16_t i = block_matrix.size() - 1 - 1; i >= 0; i--)
        {
            for (int16_t j = calculated_hashes.at(i).size() - 1; j >= 0; j--)
            {
                for (int16_t k = hashes_from_contents.at(i+1).size() - 1; k >= 0; k--)
                {
                    if (*calculated_hashes.at(i).at(j) == *hashes_from_contents.at(i+1).at(k))
                    {
                        pos.at(i).erase(pos.at(i).begin() + j);
                        std::cout << "Element found " << *calculated_hashes.at(i).at(j) << " " << *hashes_from_contents.at(i+1).at(k) << std::endl;
                        break;
                    }
                }
            }
        }

        for (int16_t l = pos.size() - 1 - 1; l >= 0 ; l--)
        {
            for (int16_t n = pos.at(l).size() - 1; n >= 0 ; n--)
            {
                std::cout << "erase p: " << pos.at(l).at(n) << std::endl;
                
                block_matrix.at(l).erase(block_matrix.at(l).begin() + pos.at(l).at(n));
                calculated_hashes.at(l).erase(calculated_hashes.at(l).begin() + pos.at(l).at(n));
                hashes_from_contents.at(l).erase(hashes_from_contents.at(l).begin() + pos.at(l).at(n));
            }
        }

        replace_block_matrix(block_matrix);
        replace_calculated_hashes(calculated_hashes);
        replace_prev_hashes(hashes_from_contents);

        // // for debugging purposes:
        // for (int i = 0; i < block_matrix.size(); i++)
        // {
        //     for (int j = 0; j < block_matrix.at(i).size(); j++)
        //     {
        //         nlohmann::json content_j = *block_matrix.at(i).at(j);
        //         std::cout << "block matrix entries " << i << " " << j << " (oldest first)" << std::endl;
        //     }
        // }
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
std::vector<std::shared_ptr<std::string>> BlockMatrix::calculated_hash_vector_;
std::vector<std::vector<std::shared_ptr<std::string>>> BlockMatrix::calculated_hash_matrix_ = {};
std::vector<std::shared_ptr<std::string>> BlockMatrix::prev_hash_vector_;
std::vector<std::vector<std::shared_ptr<std::string>>> BlockMatrix::prev_hash_matrix_ = {};
std::vector<std::shared_ptr<nlohmann::json>> BlockMatrix::received_block_vector_;
std::vector<std::vector<std::shared_ptr<nlohmann::json>>> BlockMatrix::received_block_matrix_ = {};
