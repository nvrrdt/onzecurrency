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
    class Shab58
    {
    public:
        std::vector<uint8_t> create_hash(const std::string &str);
        std::string create_base58_hash(const std::string &str);
    private:
    };
}