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
        
        cout << email_hashed << endl << password_hashed << endl;

        // sort email_hashed and password_hashed alphabetically for consistency in concateation these two hashes
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
            
            // Add new user to pool:
            merkle_tree mt;
            mt.save_new_user(user_hashed);
        }
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

void merkle_tree::save_new_user(string& new_user)
{
    ofstream file("../new_users_pool.json");
	nlohmann::json j;
	
    j.push_back(new_user);

	file << j;
}

void merkle_tree::create_block()
{
    merkle_tree::two_hours_timer();
}

void merkle_tree::two_hours_timer()
{
    using namespace std::chrono;

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

    sleep(1);

    merkle_tree::create_block();
}