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

#include <bits/stdc++.h>
#include <math.h>

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

void merkle_tree::create_block()
{
    merkle_tree::two_hours_timer();

    // parse new_users_pool.json
    ifstream file("../new_users_pool.json");
    nlohmann::json j;

    file >> j;
    
    stack <string> s;

    for (auto& element : j) {
        std::cout << std::setw(4) << element << '\n';

        // sort email_hashed and password_hashed alphabetically for consistency in concateation these two hashes
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
            cout << "root: " << user_hashed << endl;

            s.push(user_hashed);
        }
    }

    merkle_tree::calculate_root_hash(s);
}

int merkle_tree::two_hours_timer()
{
    using namespace std::chrono;
    /* TODO: remove this comment in production
    while (1)
    {
        system_clock::time_point now = system_clock::now();

        time_t tt = system_clock::to_time_t(now);
        tm utc_tm = *gmtime(&tt);
        
        if (utc_tm.tm_sec % 60 == 0 &&
            utc_tm.tm_min % 60 == 0 &&
            utc_tm.tm_hour % 2 == 0) // == 2 hours
        {
            std::cout << utc_tm.tm_hour << ":";
            std::cout << utc_tm.tm_min << endl;

            break;
        }
    }
    */
    sleep(1);

    return 0;
}

void merkle_tree::calculate_root_hash(stack <string> s)
{
    // 2^n >= s.size lambda? , then add {0}.hashed till 2^n, then pop 2 concatenate and hash, again concateante 2 and hash till root_hash
    size_t n; // 2^n

    for (size_t i = 0; i < s.size(); i++)
    {
        if (pow (2, i) >= s.size())
        {
            n = pow (2, i);
            break;
        }
    }

    size_t current_stack_size = s.size();
    for (size_t i = 0; i < (n - current_stack_size); i++)
    {
        s.push("0");
    }
    
    merkle_tree::pop_two_and_hash(s);
}

string& merkle_tree::pop_two_and_hash(stack <string> s)
{
    string uneven, even;
    stack <string> s1;

    while (!s.empty())
    {
        uneven = s.top();
        s.pop();

        even = s.top();
        s.pop();

        string parent_conc = uneven + even, parent_hashed;

        if (merkle_tree::create_hash(parent_conc, parent_hashed) == true)
        {
            s1.push(parent_hashed);
        }
    }

    if (s1.size() == 1)
    {
        string& root_hash_block = s1.top();

        cout << root_hash_block << endl;

        return root_hash_block;
    }

    merkle_tree::pop_two_and_hash(s1);
}