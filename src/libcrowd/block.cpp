#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <openssl/evp.h>

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

#include "crypto.hpp"

using namespace Crowd;

std::string merkle_tree::time_now()
{
    using namespace std::chrono;

    std::string datetime;

    system_clock::time_point now = system_clock::now();

    time_t tt = system_clock::to_time_t(now);
    tm utc_tm = *gmtime(&tt);

    datetime = to_string(utc_tm.tm_hour) + ":" + to_string(utc_tm.tm_min) + ":" + to_string(utc_tm.tm_sec) + " " \
                       + to_string(utc_tm.tm_mday) + "/" + to_string(utc_tm.tm_mon + 1) + "/" + to_string(utc_tm.tm_year + 1900);

    return datetime;
}

shared_ptr<stack<string>> merkle_tree::calculate_root_hash(shared_ptr<stack<string>> s_shptr)
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
    //std::cout << n << " " << current_stack_size << endl;
    for (size_t i = 0; i < (n - current_stack_size); i++)
    {
        string zero = "zero", zero_hashed;
        Crypto c;
        zero_hashed = c.create_base58_hash(zero);
        s_shptr->push(zero_hashed);
    }

    return merkle_tree::pop_two_and_hash(s_shptr);
}

shared_ptr<stack<string>> merkle_tree::pop_two_and_hash(shared_ptr<stack<string>> s_shptr)
{
    string uneven, even;
    shared_ptr<stack<string>> s1_shptr = make_shared<stack<string>>();

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

            string parent_conc = uneven + even, parent_hashed;

            Crypto c;
            parent_conc = c.create_base58_hash(parent_conc);
            s1_shptr->push(parent_hashed);
        }

        return merkle_tree::pop_two_and_hash(s1_shptr);
    }
}

void merkle_tree::create_block(string& datetime, string root_hash_data, nlohmann::json user_data_j)
{
    // creation of the block's data for storage
    nlohmann::json j = {
        {"starttime", datetime},
        {"hash_co", root_hash_data}
    };

    int user_count = 0;
    for (auto& element : user_data_j) {
        string full_hash, pub_key;

        full_hash = element["full_hash"];
        pub_key = element["pub_key"];

        j["data"][user_count]["email_h"].push_back(full_hash);
        j["data"][user_count]["passw_h"].push_back(pub_key);

        user_count++;
    }

    // TODO: remove these two lines
    //cout << "goed " << datetime << " " << root_hash_data << " " << user_count << endl;
    //std::cout << std::setw(4) << j << '\n';

    // hashing of the whole new block
    string block_j, block_hashed;

    // create genesis or add to blockchain
    boost::system::error_code c;
    ConfigDir cd;
    std::string blockchain_folder_path = cd.GetConfigDir() + "blockchain";

    if (!boost::filesystem::exists(blockchain_folder_path))
    {
        boost::filesystem::create_directory(blockchain_folder_path);
    }

    bool isDir = boost::filesystem::is_directory(blockchain_folder_path, c);
    bool isEmpty = boost::filesystem::is_empty(blockchain_folder_path);

    if(!isDir)
    {
        std::cout << "Error Response: " << c << std::endl;
    }
    else
    {
        if (!isEmpty)
        {
            std::cout << "Directory not empty" << std::endl;
            j["prev_hash"].push_back("prev_hash by chosen one"); // prev_hash by chosen one
            block_j = to_string(j);
            cd.CreateFileInConfigDir("blockchain/block_000000000001.json", block_j); // TODO: make it count
        }
        else
        {
            std::cout << "Is a directory, is empty" << std::endl;

            string genesis_prev_hash = "secrets are dumb, omnivalently speaking", genesis_prev_hash_hashed;

            Crypto c;
            genesis_prev_hash_hashed = c.create_base58_hash(genesis_prev_hash);
            j["prev_hash"].push_back(genesis_prev_hash_hashed);
            block_j = to_string(j);
            cd.CreateFileInConfigDir("blockchain/block_000000000000.json", block_j);
        }
    }
}
