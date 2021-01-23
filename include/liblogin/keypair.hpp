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
    class Keypair
    {
    public:
        void generate_and_save_keypair();
        std::tuple<std::vector<uint8_t>, std::vector<uint8_t>> get_existing_keypair();
        std::string get_pub_key();
    private:
        std::vector<uint8_t> pub_key_;
    };
}