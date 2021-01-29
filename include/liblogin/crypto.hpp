#pragma once

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <tuple>

#include <openssl/sha.h>
#include "key.h"
#include "base58.h"

namespace Crowd
{
    class Crypto
    {
    public:
        ecdsa::Key generate_and_save_keypair();
        ecdsa::Key get_keypair_with_priv_key(std::string priv_key);
        std::string get_priv_key();
        std::string get_pub_key();
        std::vector<uint8_t> create_hash(const std::string &str);
        std::string create_base58_hash(const std::string &str);
        std::tuple<std::vector<uint8_t>, bool> sign(const std::string string);
        bool verify(const std::string string, std::string signature_base58, std::string pub_key_base58);
    private:
    };
}