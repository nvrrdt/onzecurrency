#include <openssl/sha.h>

#include <cstdint>
#include <string>
#include <vector>

#include "base58.h"
#include "key.h"
#include "keypair.hpp"

namespace Crowd
{
    class Signature
    {
    public:
        Signature()
        {
            Keypair kp;
            std::tie(priv_key_, pub_key_) = kp.get_existing_keypair();
            ecdsa::Key key(priv_key_);
            key_ = key;
        }
        std::tuple<std::vector<uint8_t>, bool> sign(const std::string string);
        bool verify(const std::string string, std::vector<uint8_t> signature);
        std::vector<uint8_t> create_hash(const std::string &str);
    private:
        ecdsa::Key key_;
        std::vector<uint8_t> priv_key_;
        std::vector<uint8_t> pub_key_;
        std::vector<uint8_t> signature_;
        bool succ_;
    };
}