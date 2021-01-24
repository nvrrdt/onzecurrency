// #include <openssl/sha.h>

// #include <cstdint>
// #include <string>
// #include <vector>
// #include <tuple>

// #include "base58.h"
// #include "key.h"
// #include "keypair.hpp"

// using namespace Crowd;

// namespace Crowd
// {
//     class Signature
//     {
//     public:
//         Signature()
//         {
//             Keypair kp;
//             keyp_ = kp.get_existing_keypair();
//             priv_key_ = std::get<0>(keyp_);
//             pub_key_ = std::get<1>(keyp_);
//             ecdsa::Key key(priv_key_);
//             key_ = key;
//         }
//         std::tuple<std::vector<uint8_t>, bool> sign(const std::string string);
//         bool verify(const std::string string, std::vector<uint8_t> signature);
//         std::vector<uint8_t> create_hash(const std::string &str);
//         std::vector<uint8_t> get_pub_key()
//         {
//             return pub_key_;
//         }
//     private:
//         ecdsa::Key key_;
//         std::tuple<std::vector<uint8_t>, std::vector<uint8_t>> keyp_;
//         std::vector<uint8_t> priv_key_;
//         std::vector<uint8_t> pub_key_;
//         std::vector<uint8_t> signature_;
//         bool succ_;
//     };
// }