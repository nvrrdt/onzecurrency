#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "merkle_tree.hpp"

#include <fstream>
#include "json.hpp"

#include <chrono>
#include <ctime>
#include <unistd.h>

#include <math.h>
#include <memory>
#include <stack>

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include "verification.hpp"
#include "configdir.hpp"

#include "crypto.hpp"
#include "prev_hash.hpp"

#include "print_or_log.hpp"

using namespace Crowd;

std::chrono::milliseconds merkle_tree::time_now()
{
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    return now;
}

std::shared_ptr<std::stack<std::string>> merkle_tree::calculate_root_hash(std::shared_ptr<std::stack<std::string>> &s_shptr)
{
    size_t n; // 2^n

    // calculate the next 2^n above stacksize
    for (size_t i = 0; i < s_shptr->size(); i++)
    {
        if (pow (2, i) >= s_shptr->size())
        {
            n = pow (2, i);
            break;
        }
    }

    // add 0's to the stack till a size of 2^n
    size_t current_stack_size = s_shptr->size();
    if (current_stack_size == 1)
    {
        std::string parent_conc, parent_hashed;

        Common::Crypto crypto;
        parent_hashed = crypto.bech32_encode_sha256(s_shptr->top());
        s_shptr->pop();
        s_shptr->push(parent_hashed);
    }
    //std::cout << n << " " << current_stack_size << endl;
    for (size_t i = 0; i < (n - current_stack_size); i++)
    {
        std::string zero = "zero", zero_hashed;
        Common::Crypto crypto;
        zero_hashed = crypto.bech32_encode_sha256(zero);
        s_shptr->push(zero_hashed);
    }

    return merkle_tree::pop_two_and_hash(s_shptr);
}

std::shared_ptr<std::stack<std::string>> merkle_tree::pop_two_and_hash(std::shared_ptr<std::stack<std::string>> &s_shptr)
{
    std::string uneven, even;
    std::shared_ptr<std::stack<std::string>> s1_shptr = std::make_shared<std::stack<std::string>>();

    if (s_shptr->size() <= 1)
    {
        return s_shptr; // only root_hash_block is in stack s_shptr
    }
    else
    {
        while (!s_shptr->empty())
        {
            uneven = s_shptr->top(); // left!
            s_shptr->pop();

            even = s_shptr->top(); // right!
            s_shptr->pop();

            std::string parent_conc = uneven + even, parent_hashed;

            Common::Crypto crypto;
            parent_hashed = crypto.bech32_encode_sha256(parent_conc);
            s1_shptr->push(parent_hashed);
        }

        return merkle_tree::pop_two_and_hash(s1_shptr);
    }
}

nlohmann::json merkle_tree::create_block(std::string &datetime, std::string &root_hash_data, nlohmann::json &entry_data_j, nlohmann::json &exit_data_j)
{
    // creation of the block's data for storage
    nlohmann::json j = {
        {"starttime", datetime},
        {"hash_co", root_hash_data}
    };

    if (entry_data_j.empty()) j["new_entries"] = false;
    int user_count = 0;
    for (auto& element : entry_data_j) {
        std::string full_hash, ecdsa_pub_key, rsa_pub_key;

        full_hash = element["full_hash"];
        ecdsa_pub_key = element["ecdsa_pub_key"];
        rsa_pub_key = element["rsa_pub_key"];

        j["entry"][user_count]["full_hash"] = full_hash;
        j["entry"][user_count]["ecdsa_pub_key"] = ecdsa_pub_key;
        j["entry"][user_count]["rsa_pub_key"] = rsa_pub_key;

        j["new_entries"] = true;

        user_count++;
    }

    user_count = 0;
    for (auto& element : exit_data_j) { // TODO: exit strategy for users after 2 years of no transactions
        std::string full_hash;

        full_hash = element["full_hash"];

        j["exit"][user_count]["full_hash"] = full_hash;

        user_count++;
    }

    // TODO: remove these two lines
    //cout << "goed " << datetime << " " << root_hash_data << " " << user_count << endl;
    //std::cout << std::setw(4) << j << '\n';

    return j;
}

std::string merkle_tree::save_block_to_file(nlohmann::json &block_j, std::string &latest_block)
{
        // hashing of the whole new block
    std::string block_s, block_hashed;

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
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Error Response:", std::to_string(c.value())});
    }
    else
    {
        if (latest_block != "no blockchain present in folder")
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Directory not empty"});
            //PrevHash ph;
            //block_j["prev_hash"] = ph.calculate_hash_from_last_block(); // "prev_hash by chosen one"; // TODO: pull in prev_hash by chosen one !!!!!!
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
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Is a directory, is empty"});

            block_j["prev_hash"] =  get_genesis_prev_hash();
            block_s = block_j.dump();
            std::string block_file = "blockchain/crowd/block_000000000000.json";
            if (!boost::filesystem::exists(blockchain_folder_path + "/block_000000000000.json"))
            {
                cd.CreateFileInConfigDir(block_file, block_s);
            }
        }
    }

    return block_s;
}

std::string merkle_tree::get_genesis_prev_hash()
{
    std::string genesis_message = "secrets are dumb, omnivalently speaking", genesis_prev_hash;
    Common::Crypto crypto;
    return crypto.bech32_encode_sha256(genesis_message);
}