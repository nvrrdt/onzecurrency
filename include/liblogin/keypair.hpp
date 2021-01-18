#pragma once

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include <openssl/sha.h>
#include "key.h"
#include "base58.h"

namespace Crowd
{
    class Keypair
    {
    public:
        void generate_and_save_keypair();
    private:
    };
}