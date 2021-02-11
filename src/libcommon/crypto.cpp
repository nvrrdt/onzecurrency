#include "crypto.hpp"
#include "base58.hpp"

#include "cryptlib.h"
#include "sha3.h"
#include "filters.h"

using namespace Crowd;
using namespace CryptoPP;
using namespace base58;

std::string Crypto::create_sha256(std::string &msg)
{
    std::string digest;

    SHA3_256 hash;
    StringSource(msg, true, new HashFilter(hash, new StringSink(digest)));

    return digest;
}

std::string Crypto::base58_encode(std::string &msg)
{
    std::string str = create_sha256(msg);
    std::vector<uint8_t> vec(str.begin(), str.end());
    return base58::EncodeBase58(vec);
}

std::string Crypto::base58_decode(std::string &b58)
{
    std::vector<uint8_t> vec;

    if (base58::DecodeBase58(b58, vec))
    {
        std::string str(vec.begin(), vec.end());
        return str;
    }
    else
    {
        return "false";
    }
}

// ecdsa and rsa next