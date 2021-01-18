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

std::string Keypair::get_pub_key()
{
    return base58::EncodeBase58(pub_key_);
}