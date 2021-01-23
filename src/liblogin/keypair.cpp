#include <vector>
#include <string>

#include "keypair.hpp"
#include "configdir.hpp"

using namespace Crowd;

void Keypair::generate_and_save_keypair()
{
    // generate keys:
    ecdsa::Key key;

    auto priv_key = key.get_priv_key_data();
    key.CreatePubKey();
    pub_key_ = key.get_pub_key_data();
    //std::cout << "priv_key: " << base58::EncodeBase58(priv_key) << ", pub_key: " << base58::EncodeBase58(pub_key_) << std::endl;

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
        cd.CreateFileInConfigDir("pub_key", base58::EncodeBase58(pub_key_));
    }
    else
    {
        std::cout << "Pub_key existed already!!" << std::endl;
    }
}

std::tuple<std::vector<uint8_t>, std::vector<uint8_t>> Keypair::get_existing_keypair()
{
    std::vector<uint8_t> priv_key;

    // read priv_key file
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "priv_key"))
    {
        // get priv_key
        std::ifstream stream(cd.GetConfigDir() + "priv_key", std::ios::in | std::ios::binary);
        std::vector<uint8_t> contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        for(auto i: contents) {
            int value = i;
            std::cout << "data: " << value << std::endl;
        }

        priv_key = contents;
    }
    else
    {
        std::cout << "Priv_key existed already!!" << std::endl;
    }

    if (boost::filesystem::exists(cd.GetConfigDir() + "pub_key"))
    {
        // get pub_key
        std::ifstream stream(cd.GetConfigDir() + "pub_key", std::ios::in | std::ios::binary);
        std::vector<uint8_t> contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        for(auto i: contents) {
            int value = i;
            std::cout << "data: " << value << std::endl;
        }

        pub_key_ = contents;
    }
    else
    {
        std::cout << "Pub_key existed already!!" << std::endl;
    }

    return std::make_tuple(priv_key, pub_key_);
}

std::string Keypair::get_pub_key()
{
    return base58::EncodeBase58(pub_key_);
}