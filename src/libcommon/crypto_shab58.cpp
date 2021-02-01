#include <vector>
#include <string>

#include "crypto_shab58.hpp"
#include "configdir.hpp"

using namespace Crowd;

std::vector<uint8_t> Shab58::create_hash(const std::string &str)
{
  SHA512_CTX ctx;
  SHA512_Init(&ctx);
  SHA512_Update(&ctx, str.c_str(), str.size());
  std::vector<uint8_t> md(SHA512_DIGEST_LENGTH);
  SHA512_Final(md.data(), &ctx);
  return md;
}

std::string Shab58::create_base58_hash(const std::string &str)
{
    return base58::EncodeBase58(create_hash(str));
}
