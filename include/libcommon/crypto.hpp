#pragma once

#include <iostream>
#include <string>

namespace Crowd
{
    class Crypto
    {
    public:
        std::string create_sha256(std::string &msg);
        std::string base58_encode(std::string &msg);
        std::string base58_decode(std::string &b58);
        // ecdsa::Key generate_and_save_keypair();
        // ecdsa::Key get_keypair_with_priv_key(std::string &priv_key);
        // std::string get_priv_key();
        // std::string get_pub_key();
        // std::tuple<std::vector<uint8_t>, bool> sign(const std::string &string);
        // bool verify(const std::string &string, std::string &signature_base58, std::string &pub_key_base58);
    private:
    };
}