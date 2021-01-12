#pragma once

#include <string>
#include <iostream>
#include "crypto.h"
#include "base64.h"

namespace Crowd
{
    class CryptoMain
    {
    public:
        CryptoMain(std::string hash_of_peer) {
            hash_of_peer_ = hash_of_peer;
        }
        void do_encrypt_rsa_message();
        void do_decrypt_rsa_message();
        std::string do_encrypt_aes_message();
        void do_decrypt_aes_message();
    private:
        std::string hash_of_peer_;
    };
}