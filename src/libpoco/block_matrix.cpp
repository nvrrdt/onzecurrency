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

std::cout << "1106 kl " << get_received_block_matrix().empty() << std::endl;
std::cout << "1106 kl " << get_received_block_matrix().size() << std::endl;
    if (!get_block_matrix().empty() && !get_received_block_matrix().empty())
    {
        // remove non-received blocks in the last block_vector of block_matrix
        for (uint16_t i = 0; i < get_block_matrix().back().size(); i++)
        {
            if (get_received_block_matrix().back().empty()) break;

            bool found = false;

            std::vector<std::shared_ptr<nlohmann::json>>::iterator it;
            it = get_block_matrix().back().begin() + i;
std::cout << "1107" << std::endl;
            for (uint16_t j = 0; j < get_received_block_matrix().back().size(); j++)
            {
                // TODO create hashes ... maybe
std::cout << "1107 kl kl" << std::endl;                
                if (*get_block_matrix().back().at(i) == *get_received_block_matrix().back().at(j))
                {
std::cout << "1107 found found" << std::endl;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
std::cout << "1108 kl1" << std::endl;
std::cout << "1108 kl empty " << get_block_matrix().back().size() << std::endl;
                if (!get_received_block_matrix().back().empty()) get_block_matrix().back().erase(it);
std::cout << "1108 kl2" << std::endl;
                continue;
            }
        }
std::cout << "1109" << std::endl;
        get_received_block_matrix().back().clear();
    }

    /**
     * last vector block_matrix:
     * create list of all prev_hashes in those files,
     * last - 1 vector block_matrix:
     * create list of calculated prev_hashes,
     * compare two lists and delete in last - 1 the blocks that aren't in last,
     * then compare last - 2 and last -1, ...,
     * just until the final block remains
     */

    if (!get_block_matrix().empty())
    {
        // get prev_hashes from within the latest vector and compare with the hashes from before latest vector
        auto calculated_hashes = get_calculated_hash_matrix();
        auto hashes_from_contents = get_prev_hash_matrix();

        std::cout << "evaluate_both_block_matrices m " << get_block_matrix().size() << std::endl;
        std::cout << "evaluate_both_block_matrices v " << get_block_matrix().back().size() << std::endl;
        std::cout << "evaluate_both_block_matrices mcph " << get_calculated_hash_matrix().size() << std::endl;
        std::cout << "evaluate_both_block_matrices vcph " << get_calculated_hash_matrix().back().size() << std::endl;
        std::cout << "evaluate_both_block_matrices mphfc " << get_prev_hash_matrix().size() << std::endl;
        std::cout << "evaluate_both_block_matrices vphfc " << get_prev_hash_matrix().back().size() << std::endl;

        // std::cout << "evaluate_both_block_matrices rm " << get_received_block_matrix().size() << std::endl;
        // std::cout << "evaluate_both_block_matrices rv " << get_received_block_matrix().back().size() << std::endl;

//         for (uint16_t i = 0; i < calculated_hashes.size(); i++)
//         {
// std::cout << "000000 " << calculated_hashes.size() << std::endl;
//             if (i == calculated_hashes.size() - 1) continue;
// std::cout << "000000 " << std::endl;
//             for (uint16_t j = 0; j < calculated_hashes.at(i).size(); j++)
//             {
// std::cout << "111111 " << std::endl;
//                 calculated_hashes.at(i).erase(std::remove_if(calculated_hashes.at(i).begin(), 
//                                                              calculated_hashes.at(i).end(),
//                                                              [](auto str){
//                                                                  std::cout << "beestigheid________ " << *str << std::endl;
//                                                                  return true;
//                                                              }),
//                                               calculated_hashes.at(i).end());
//             }
//         }

    // for debugging purposes:
    for (int x = 0; x < calculated_hashes.size(); x++)
    {
        for (int y = 0; y < calculated_hashes.at(x).size(); y++)
        {
            std::string content = *calculated_hashes.at(x).at(y);
            std::cout << "calculated_hashes matrix entry prel " << x << " " << y << " (oldest first) " << content << std::endl << std::endl;
        }
    }

    // for debugging purposes:
    for (int v = 0; v < hashes_from_contents.size(); v++)
    {
        for (int w = 0; w < hashes_from_contents.at(v).size(); w++)
        {
            std::string content = *hashes_from_contents.at(v).at(w);
            std::cout << "hashes_from_contents matrix entry prel " << v << " " << w << " (oldest first) " << content << std::endl << std::endl;
        }
    }

        for (int16_t i = get_block_matrix().size() - 1 - 1; i >= 0; i--)
        {
std::cout << "00000 " << std::endl;
            std::vector<int16_t> pos = {};
            for (int16_t k = 0; k < get_block_matrix().at(i).size(); k++)
            {
                pos.push_back(k);
            }
std::cout << "00001 " << std::endl;
            for (int16_t j = calculated_hashes.at(i).size() - 1; j >= 0; j--)
            {
                for (int16_t k = hashes_from_contents.at(i+1).size() - 1; k >= 0; k--)
                {
                    if (*calculated_hashes.at(i).at(j) == *hashes_from_contents.at(i+1).at(k))
                    {
                        pos.erase(pos.begin() + j);
                        std::cout << "Element found" << *calculated_hashes.at(i).at(j) << " " << *hashes_from_contents.at(i+1).at(k) << " k " << k << " pos " << pos[j] << std::endl;
                        break;
                    }
                }

                break;
            }
            // for (int16_t j = get_block_matrix().at(i+1).size() - 1; j >= 0; j--)
            // {
            //     auto it = std::find_if(calculated_hashes.at(i).begin(), calculated_hashes.at(i).end(), 
            //                             [&](std::shared_ptr<std::string> calculated_hash_ptr){
            //                                 //std::cout << "kerel " << *hash_from_contents_ptr << " " << *calculated_hashes.at(i).at(j) << std::endl;
            //                                 return *calculated_hash_ptr == *hashes_from_contents.at(i+1).at(j);});
// std::cout << "00001 " << *calculated_hashes.at(i).at(0) << " " << *hashes_from_contents.at(i+1).at(0) << std::endl;
//                 if (it != calculated_hashes.at(i).end())
//                 {
// //                     int index = it - calculated_hashes.at(i).begin();
// // std::cout << "00002 " << std::endl;
// //                     pos.erase(pos.begin() + index);
// // std::cout << "00003 " << std::endl;
//                     //pos.push_back(index);
//                     std::cout << "Element found " << std::endl;
//                     // break;
//                 }
//                 else
//                 {
//                     int index = it - calculated_hashes.at(i).begin();
//                     pos.push_back(index);
                    
//                     std::cout << "Element not found " << index << std::endl;
//                 }
//             }

            for (int16_t l = pos.size() - 1; l >= 0 ; l--)
            {
                int p = pos[l];
std::cout << "33333 12 p: " << p << std::endl;
                get_block_matrix().at(i)[p] = get_block_matrix().at(i).back();
                get_block_matrix().at(i).pop_back();

                calculated_hashes.at(i)[p] = calculated_hashes.at(i).back();
                calculated_hashes.at(i).pop_back();
                
                hashes_from_contents.at(i)[p] = hashes_from_contents.at(i).back();
                hashes_from_contents.at(i).pop_back();
                
                // calculated_hashes.at(i).erase(calculated_hashes.at(i).begin() + pos[l]);
                // hashes_from_contents.at(i).erase(hashes_from_contents.at(i).begin() + pos[l]);
std::cout << "33333 13" << std::endl;
            }

//             for (int16_t j = get_block_matrix().at(i).size() - 1; j >= 0; j--)
//             {
// std::cout << "11111 " << std::endl;
//                 for (int16_t k = get_block_matrix().at(i+1).size() - 1; k >= 0; k--)
//                 {
// std::cout << "22222 j " << get_block_matrix().at(i).size() << std::endl;
// std::cout << "22222 k " << get_block_matrix().at(i+1).size() << std::endl;
//                     if (*calculated_hashes.at(i).at(j) == *hashes_from_contents.at(i+1).at(k))
//                     {
// std::cout << "33333 " << j << std::endl;
//                         pos.push_back(j);
//                         break;
//                     }
//                 }
//             }
// std::cout << "33333 11" << std::endl;
//             for (int16_t l = 0; l < pos.size(); l++)
//             {
// std::cout << "33333 12 " << l << " p " << pos[l] << std::endl;
//                 get_block_matrix().at(i)[l] = get_block_matrix().at(i).back();
//                 get_block_matrix().at(i).pop_back();

//                 calculated_hashes.at(i)[l] = calculated_hashes.at(i).back();
//                 calculated_hashes.at(i).pop_back();
                
//                 hashes_from_contents.at(i)[l] = hashes_from_contents.at(i).back();
//                 hashes_from_contents.at(i).pop_back();
                
                // calculated_hashes.at(i).erase(calculated_hashes.at(i).begin() + pos[l]);
                // hashes_from_contents.at(i).erase(hashes_from_contents.at(i).begin() + pos[l]);
// std::cout << "33333 13" << std::endl;
            // }
        }
std::cout << "44444 " << std::endl;



// if (std::find(v.begin(), v.end(), key) != v.end()) {
//     std::cout << "Element found";
// }
// else {
//     std::cout << "Element not found";
// }



// void quickDelete( int idx )
// {
//   vec[idx] = vec.back();
//   vec.pop_back();
// }


// auto pos = container.begin();
// for(auto i = container.begin(); i != container.end(); ++i)
// {
//     if(isKeepElement(*i)) // whatever condition...
//     {
//         *pos++ = *i; // will move, if move assignment is available...
//     }
// }
// // well, std::remove(_if) stops here...
// container.erase(pos, container.end());





// std::cout << "1112 1 " << ph.get_prev_hashes_from_block_matrix_contents().size() << std::endl;
//         for (uint16_t i = ph.get_prev_hashes_from_block_matrix_contents().size() - 1; i >= 0; i--)
//         {
// std::cout << "1112 2 " << ph.get_prev_hashes_from_block_matrix_contents().at(i).size() << std::endl;
//             if (i == 0) continue;


// // all those matrixes are equal in size, zo two for loops for each vector to be able to compare


//             for (uint16_t j = ph.calculate_hashes_from_block_matrix().at(i-1).size() - 1; j >= 0; j--)
//             {
//                 if (j > ph.calculate_hashes_from_block_matrix().at(i).size()) break; // otherwise it goes to 65535

//                 bool found = false;

//                 for (uint16_t k = 0; k < ph.get_prev_hashes_from_block_matrix_contents().at(i).size(); k++)
//                 {
// std::cout << "1114 1 i:" << i << ", j: " << j << ", k: " << k << std::endl;
//                     std::string calculated_prev_hash = *ph.calculate_hashes_from_block_matrix().at(i-1).at(j); // j moet 0 zijn op i = 0
// std::cout << "1114 2" << std::endl;
//                     std::string prev_hash_from_contents = *ph.get_prev_hashes_from_block_matrix_contents().at(i).at(k);
// std::cout << "cph " << calculated_prev_hash << std::endl;
// std::cout << "phfc " << prev_hash_from_contents << std::endl;
//                     if (calculated_prev_hash == prev_hash_from_contents)
//                     {
//                         found = true;
//                         break;
//                     }
//                 }

// std::cout << "1115" << std::endl;
//                 if (!found)
//                 {
//                     // all next blocks in i - 1 must be deleted

// std::cout << "1115 1" << std::endl;
//                     get_block_matrix().at(i-1).erase(get_block_matrix().at(i-1).begin()+j);
// std::cout << "1115 2" << std::endl;
//                     ph.get_prev_hashes_from_block_matrix_contents().at(i-1).erase(ph.get_prev_hashes_from_block_matrix_contents().at(i-1).begin()+j);
// std::cout << "1115 3" << std::endl;
//                     ph.calculate_hashes_from_block_matrix().at(i-1).erase(ph.calculate_hashes_from_block_matrix().at(i-1).begin()+j);
//                     continue;
//                 }
//             }
// std::cout << "1116" << std::endl;
//         }
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
