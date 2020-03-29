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
#include "p2p_handler.hpp"

using namespace crowd;

/**
 * a block consists of a timestamp hashed just before the root hash and a merkle tree of users
 * or a list of alfabetically sorted users and a root hash
 * each user is hashed email concatenatenated with a hashed password
 * when logging in the hashed email and hashed password must be found in the verified blockchain
 * logging in with email and password and blockchain id, the last one is for trying to make a private blockchain
 * the timestamp is the start of a timeframe of 1 hour
 */



void merkle_tree::create_user(string email, string password)
{
    string email_hashed, password_hashed, user_conc, user_hashed;

    if (merkle_tree::create_hash(email, email_hashed) &&
        merkle_tree::create_hash(password, password_hashed) == true)
    {
        // Add new user's credentials to pool:
        merkle_tree::save_new_user(email_hashed, password_hashed);
    }
}

bool merkle_tree::create_hash(const string& unhashed, string& hashed)
{
    bool success = false;

    EVP_MD_CTX* context = EVP_MD_CTX_new();

    if(context != NULL)
    {
        if(EVP_DigestInit_ex(context, EVP_sha256(), NULL))
        {
            if(EVP_DigestUpdate(context, unhashed.c_str(), unhashed.length()))
            {
                unsigned char hash[EVP_MAX_MD_SIZE];
                unsigned int lengthOfHash = 0;

                if(EVP_DigestFinal_ex(context, hash, &lengthOfHash))
                {
                    std::stringstream ss;
                    for(unsigned int i = 0; i < lengthOfHash; ++i)
                    {
                        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
                    }

                    hashed = ss.str();
                    success = true;
                }
            }
        }

        EVP_MD_CTX_free(context);
    }

    return success;
}

void merkle_tree::save_new_user(string& hashed_email_new_user, string& hashed_password_new_user)
{
    ifstream ifile("../new_users_pool.json", ios::in);

    nlohmann::json j;

    if (merkle_tree::is_empty(ifile))
    {
        ofstream ofile("../new_users_pool.json", ios::out);
        
        ofile.clear();

        nlohmann::json i;
        i.push_back(hashed_email_new_user);
        i.push_back(hashed_password_new_user);

        j.push_back(i);

        ofile << j;
    }
    else
    {
        ofstream ofile("../new_users_pool.json", ios::out);

        ifile >> j;
        ofile.clear();

        nlohmann::json i;

        i.push_back(hashed_email_new_user);
        i.push_back(hashed_password_new_user);
    
        j += i;

        ofile << j;
    }
}

bool merkle_tree::is_empty(std::ifstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}

int merkle_tree::prep_block_creation()
{
    string datetime = merkle_tree::two_hours_timer();

    // parse new_users_pool.json
    ifstream file("../new_users_pool.json");
    nlohmann::json j;

    file >> j;
    
    // return if new_users_pool.json is empty
    if (j.empty()) {
        return 1;
    }

    shared_ptr<stack<string>> s_shptr = make_shared<stack<string>>();

    for (auto& element : j) {
        //std::cout << std::setw(4) << element << '\n';

        // sort email_hashed and password_hashed alphabetically for consistency in concatenating these two hashes
        string email_hashed, password_hashed, user_conc, user_hashed;
        // TODO: the user's of the block, only hashed email and hashed password, need also to be stored in a json with block number
        // but there's block arithmetic needed when the winning block is smaller and bigger than this one calculated here ...
        email_hashed = element[0];
        password_hashed = element[1];
        if (email_hashed <= password_hashed)
        {
            user_conc = email_hashed + password_hashed;
        }
        else 
        {
            user_conc = password_hashed + email_hashed;
        }

        if (merkle_tree::create_hash(user_conc, user_hashed) == true)
        {
            //std::cout << "root: " << string(user_hashed) << endl;

            s_shptr->push(user_hashed);
        }
    }

    s_shptr = merkle_tree::calculate_root_hash(s_shptr);

    //std::cout << "root hash block: " << s_shptr->top() << endl;

    merkle_tree::create_block(datetime, s_shptr->top(), j);

    // TODO: setup the block and and add to the blockchain, see the text in the beginning of this file for missing information

    return 0;
}

string merkle_tree::two_hours_timer()
{
    using namespace std::chrono;

    string datetime;

    while (1)
    {
        system_clock::time_point now = system_clock::now();

        time_t tt = system_clock::to_time_t(now);
        tm utc_tm = *gmtime(&tt);
        
        /** TODO: remove this comment in production
        if (utc_tm.tm_sec % 60 == 0 &&
            utc_tm.tm_min % 60 == 0 &&
            utc_tm.tm_hour % 2 == 0) // == 2 hours */
        if (utc_tm.tm_sec % 60 == 0 ||
            utc_tm.tm_sec % 60 == 20 ||
            utc_tm.tm_sec % 60 == 40)
        {
            datetime = to_string(utc_tm.tm_hour) + ":" + to_string(utc_tm.tm_min) + ":" + to_string(utc_tm.tm_sec) + " " \
                       + to_string(utc_tm.tm_mday) + "/" + to_string(utc_tm.tm_mon + 1) + "/" + to_string(utc_tm.tm_year + 1900);

            break;
        }
    }

    sleep(1);

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
        if (merkle_tree::create_hash(zero, zero_hashed) == true)
        {
            s_shptr->push(zero_hashed);
        }
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

            if (merkle_tree::create_hash(parent_conc, parent_hashed) == true)
            {
                s1_shptr->push(parent_hashed);
            }
        }

        return merkle_tree::pop_two_and_hash(s1_shptr);
    }
}

void merkle_tree::create_block(string& datetime, string root_hash_data, nlohmann::json user_data_j)
{
    /**
     * in block: put in time since epoch when block is initiated (eg. 02:00:00 or 16:00:00),
     * the hashes (not root hashes!!) from the users (data) the previous block's root hash and the hash from the chosen one
     * 
     * a verification of next prev_hash follows, if approved/sealed/confirmed by the chosen one, then the next prev_hash get's distributed
     * 
     * create file with incremental (+1) numbers in blockchain folder: 'block000000000.json'
     * 
     * a prize for when the hash's data remains a secret costs a lot of computing energy, so no, no prize to unhash
     * the genesis's data is 'secrets are dumb'
     */

    // creation of the block's data for storage
    nlohmann::json j = {
        {"starttime", datetime},
        {"hash_co", root_hash_data}
    };

    int user_count = 0;
    for (auto& element : user_data_j) {
        string email_hashed, password_hashed;

        email_hashed = element[0];
        password_hashed = element[1];

        j["data"][user_count]["email_h"].push_back(email_hashed);
        j["data"][user_count]["passw_h"].push_back(password_hashed);

        user_count++;
    }

    // TODO: remove these two lines
    //cout << "goed " << datetime << " " << root_hash_data << " " << user_count << endl;
    //std::cout << std::setw(4) << j << '\n';

    // hashing of the whole new block
    string block_j, block_hashed;

    // create genesis or add to blockchain
    boost::system::error_code c;
    boost::filesystem::path path("../blockchain");
    bool isDir = boost::filesystem::is_directory(path, c);
    bool isEmpty = boost::filesystem::is_empty(path);

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
            merkle_tree::add_block_to_blockchain(block_j);
        }
        else
        {
            std::cout << "Is a directory, is empty" << std::endl;

            string genesis_prev_hash = "secrets are dumb, omnivalently speaking", genesis_prev_hash_hashed;

            if (merkle_tree::create_hash(genesis_prev_hash, genesis_prev_hash_hashed) == true)
            {
                j["prev_hash"].push_back(genesis_prev_hash_hashed);
                block_j = to_string(j);
                merkle_tree::create_genesis_block(block_j, user_data_j);
            }
        }
    }

    // delete contents of new_users_pool.json
    std::ofstream ofs;
    ofs.open("../new_users_pool.json", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
 
    // TODO: dunno yet ... what to do with it
    if (merkle_tree::create_hash(block_j, block_hashed) == true)
    {
        // TODO:
        // search and find the hash or the hash_above in binary search tree (map) of users, that's the chosen_one
        // (there need to be already a blockchain, so eventually you're working in the dark (hypothetically!))
        // thus, create a hash table first
        // then let the chosen_one hash it, and distribute and confirm the block_hashed of his/hers peers^1 and peers^2
        // then save the json to the blockchain folder as previously described above
        // inception or genesis user or bootstrapping this is another thing

        // FOR NOW:
        // if new_users_pool.json !empty, check if blockchain folder is empty
        // if empty create genesis block and genesis user(-s)
        // if not empty add new block to blockchain

        // TODO: do something with block_hashed
    }
}

void merkle_tree::create_genesis_block(string block, nlohmann::json user_data_j)
{
    /**
     * REMARQUE: the users of the genesis block need to be online for 2 hours after logging in
     */

    //cout << "create genesis block " << block << endl;

    string block_hashed; // block_hashed is the hash as base for finding the chosen one
    std::map<std::string, std::string> map_of_users;
    if (merkle_tree::create_hash(block, block_hashed) == true)
    {
        // TODO: create map first, find upper_bound in map, that's the chosen one, let him/her communicate block_hashed to his/hers peers^1 en peers^2
        // --> Use block to create the hashes from the users and the map to find the chosen_one

        //cout << "heel " << to_string(user_data_j) << endl;
        for (auto& element : user_data_j)
        {
            //cout << "hele " << element << endl;
            //cout << "hele1 " << element[0] << endl;
            //cout << "hele2 " << element[1] << endl;
            string user_conc = string(element[0]) + string(element[1]), user_conc_hashed;
            if (merkle_tree::create_hash(user_conc, user_conc_hashed) == true)
            {
                //cout << "heleconc " << user_conc << " " << user_conc_hashed << endl;
                map_of_users.insert(std::make_pair(user_conc_hashed, "not_online"));
            }
        }
    }

    string chosen_one, ip_chosen_one;

    // find the chosen_one or poco
    for (std::map<std::string, std::string>::iterator it=map_of_users.begin(); it!=map_of_users.end(); ++it)
    {
        if (block_hashed >= it->first)
        {
            // jeej chosen_one found
            //cout << "chosen_one is " << it->first << endl;
            chosen_one = it->first;
            ip_chosen_one = it->second;
        }
        else if (it == --map_of_users.end() && block_hashed != it->first)
        {
            //cout << "end of genesis map" << it->first << endl;
            // chosen_one is the first entry of the map
            //cout << "chosen_one in begin of map " << map_of_users.begin()->first << endl;
            chosen_one = map_of_users.begin()->first;
            ip_chosen_one = map_of_users.begin()->second;
        }
    }

    /**
     * TODO: let the chosen_one verify the genesis block's hash, if true, communicate to your peers and create the block on disk if 51% of online users confirms
     *       if not true, stop genesis block creation
     * 
     * - put ip_adress in map instead of ONLINE in the value
     * - 51% of all online users must confirm the block's hash to the chosen_one
     */

    ::break_server_loop = true; // break the server loop to enable the client again
    p2p_handler ph;
    string msg = "verify"; // verify by chosen_one
    string * m = &msg;
    ph.p2p_switch(*m, ip_chosen_one); // TODO: overload this function in p2p_handler.cpp, false should disappear

    // create the block on disk
    ofstream ofile("../blockchain/block0000000000.json", ios::out | ios::trunc);

    ofile << block;
    ofile.close();
}

void merkle_tree::add_block_to_blockchain(string block)
{
    cout << "add block to blockchain" << endl;
}
