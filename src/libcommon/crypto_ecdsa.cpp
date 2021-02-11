// #include "crypto_ecdsa.hpp"
// #include "configdir.hpp"
// #include "crypto_shab58.hpp"

// using namespace Crowd;

// ecdsa::Key Ecdsa::generate_and_save_keypair()
// {
//     // generate keys:
//     ecdsa::Key key;

//     auto priv_key = key.get_priv_key_data();
//     key.CreatePubKey();
//     auto pub_key = key.get_pub_key_data();
//     //std::cout << "priv_key: " << base58::EncodeBase58(priv_key) << ", pub_key: " << base58::EncodeBase58(pub_key) << std::endl;

//     // save keys:
//     ConfigDir cd;
//     if (!boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_priv_key"))
//     {
//         std::string file = "ecdsa_priv_key";
//         std::string contents = base58::EncodeBase58(priv_key);
//         cd.CreateFileInConfigDir(file, contents);
//     }
//     else
//     {
//         std::cout << "Ecdsa_priv_key existed already!!" << std::endl;
//     }
    
//     if (!boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_pub_key"))
//     {
//         std::string file = "ecdsa_pub_key";
//         std::string contents = base58::EncodeBase58(pub_key);
//         cd.CreateFileInConfigDir(file, contents);
//     }
//     else
//     {
//         std::cout << "Ecdsa_pub_key existed already!!" << std::endl;
//     }

//     return key;
// }

// ecdsa::Key Ecdsa::get_keypair_with_priv_key(std::string &priv_key)
// {
//     std::vector<uint8_t> priv_key_data;
//     base58::DecodeBase58(priv_key, priv_key_data);
//     ecdsa::Key key(priv_key_data);
//     return key;
// }

// std::string Ecdsa::get_priv_key()
// {
//     std::string priv_key;

//     // read priv_key file
//     ConfigDir cd;
//     if (boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_priv_key"))
//     {
//         // get priv_key
//         std::ifstream stream(cd.GetConfigDir() + "ecdsa_priv_key", std::ios::in | std::ios::binary);
//         std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

//         for(auto i: contents) {
//             int value = i;
//             //std::cout << "data: " << value << std::endl;
//         }

//         priv_key = contents;
//     }
//     else
//     {
//         std::cout << "Ecdsa_priv_key existed already!!" << std::endl;
//     }

//     return priv_key;
// }

// std::string Ecdsa::get_pub_key()
// {
//     std::string pub_key;

//     // read pub_key file
//     ConfigDir cd;
//     if (boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_pub_key"))
//     {
//         // get pub_key
//         std::ifstream stream(cd.GetConfigDir() + "ecdsa_pub_key", std::ios::in | std::ios::binary);
//         std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

//         for(auto i: contents) {
//             int value = i;
//             //std::cout << "data: " << value << std::endl;
//         }

//         pub_key = contents;
//     }
//     else
//     {
//         std::cout << "Ecdsa_pub_key existed already!!" << std::endl;
//     }

//     return pub_key;
// }

// std::tuple<std::vector<uint8_t>, bool> Ecdsa::sign(const std::string &string)
// {
//     auto priv_key = Ecdsa::get_priv_key();
//     auto key = Ecdsa::get_keypair_with_priv_key(priv_key);
//     Shab58 s;
//     return key.Sign(s.create_hash(string));
// }

// bool Ecdsa::verify(const std::string &string, std::string &signature_base58, std::string &pub_key_base58)
// {
//     std::vector<uint8_t> pub_key_data, signature;
//     base58::DecodeBase58(pub_key_base58, pub_key_data);
//     ecdsa::PubKey pub_key(pub_key_data);
//     base58::DecodeBase58(signature_base58, signature);
//     Shab58 s;
//     return pub_key.Verify(s.create_hash(string), signature);
// }