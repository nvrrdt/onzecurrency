#include <vector>
#include <string>

#include "crypto.hpp"
#include "configdir.hpp"

using namespace Crowd;

ecdsa::Key Crypto::generate_and_save_keypair()
{
    // generate keys:
    ecdsa::Key key;

    auto priv_key = key.get_priv_key_data();
    key.CreatePubKey();
    auto pub_key = key.get_pub_key_data();
    //std::cout << "priv_key: " << base58::EncodeBase58(priv_key) << ", pub_key: " << base58::EncodeBase58(pub_key) << std::endl;

    // save keys:
    ConfigDir cd;
    if (!boost::filesystem::exists(cd.GetConfigDir() + "priv_key"))
    {
        cd.CreateFileInConfigDir("priv_key", base58::EncodeBase58(priv_key));
    }
    else
    {
        std::cout << "Priv_key existed already!!" << std::endl;
    }
    
    if (!boost::filesystem::exists(cd.GetConfigDir() + "pub_key"))
    {
        cd.CreateFileInConfigDir("pub_key", base58::EncodeBase58(pub_key));
    }
    else
    {
        std::cout << "Pub_key existed already!!" << std::endl;
    }

    return key;
}

ecdsa::Key Crypto::get_keypair_with_priv_key(std::string priv_key)
{
    std::vector<uint8_t> priv_key_data;
    base58::DecodeBase58(priv_key, priv_key_data);
    ecdsa::Key key(priv_key_data);
    return key;
}

std::string Crypto::get_priv_key()
{
    std::string priv_key;

    // read priv_key file
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "priv_key"))
    {
        // get priv_key
        std::ifstream stream(cd.GetConfigDir() + "priv_key", std::ios::in | std::ios::binary);
        std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        for(auto i: contents) {
            int value = i;
            //std::cout << "data: " << value << std::endl;
        }

        priv_key = contents;
    }
    else
    {
        std::cout << "Priv_key existed already!!" << std::endl;
    }

    return priv_key;
}

std::string Crypto::get_pub_key()
{
    std::string pub_key;

    // read pub_key file
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "pub_key"))
    {
        // get pub_key
        std::ifstream stream(cd.GetConfigDir() + "pub_key", std::ios::in | std::ios::binary);
        std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        for(auto i: contents) {
            int value = i;
            //std::cout << "data: " << value << std::endl;
        }

        pub_key = contents;
    }
    else
    {
        std::cout << "Pub_key existed already!!" << std::endl;
    }

    return pub_key;
}

std::vector<uint8_t> Crypto::create_hash(const std::string &str)
{
  SHA512_CTX ctx;
  SHA512_Init(&ctx);
  SHA512_Update(&ctx, str.c_str(), str.size());
  std::vector<uint8_t> md(SHA512_DIGEST_LENGTH);
  SHA512_Final(md.data(), &ctx);
  return md;
}

std::string Crypto::create_base58_hash(const std::string &str)
{
    return base58::EncodeBase58(create_hash(str));
}

std::tuple<std::vector<uint8_t>, bool> Crypto::sign(const std::string string)
{
    auto key = Crypto::get_keypair_with_priv_key(Crypto::get_priv_key());
    return key.Sign(create_hash(string));
}

bool Crypto::verify(const std::string string, std::vector<uint8_t> signature)
{
    // auto pub_key = get_pub_key(); // TODO get_pub_key from blockchain
    // pub_key.Verify(Hash(string) == signature ? true : false;
    return true;
}