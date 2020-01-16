#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
// #include <openssl/evp.h> // add this line when evp with new libssl

#include <openssl/sha.h> // remove this when using evp with new libssl

#include "merkle_tree.hpp"

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
    // de hash van de user's emailadressen en de hash van het paswoord concatenated
    // in main moet nog 2 inputs komen voor emailadres en paswoord

    cout << create_hash(email) << endl;
    cout << create_hash(password) << endl;
}

// remove below function with newe libssl version
string merkle_tree::create_hash(const string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

/* needs to be used with a newer version of libssl
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
*/