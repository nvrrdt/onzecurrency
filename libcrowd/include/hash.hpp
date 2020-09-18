#pragma once

#include <iostream>
#include <string>
#include <openssl/evp.h>
#include <iomanip>

namespace Crowd
{
    /**
     * Create a SHA256 hash
     */

    class Hash
    {
    public:
        bool create_hash(const std::string& unhashed, std::string& hashed)
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
    };
}
