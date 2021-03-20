#include "poco.hpp"
#include "p2p.hpp"
#include "auth.hpp"
#include "crypto.hpp"

using namespace Crowd;

void Poco::inform_chosen_ones(std::string my_latest_block_nr, nlohmann::json block_j)
{
    Auth a;
    std::string my_full_hash = a.get_my_full_hash();
    Crypto* crypto = new Crypto();
    std::string block_s = block_j.dump();
    std::string hash_of_block = crypto->bech32_encode_sha256(block_s);
    delete crypto;
    Rocksy* rocksy = new Rocksy();
    std::string co_from_this_block = rocksy->FindChosenOne(hash_of_block);
    delete rocksy;

    if (co_from_this_block == my_full_hash)
    {
        // You are the coordinator!
        std::cout << "Inform my fellow chosen_ones as coordinator" << std::endl;

        Protocol proto;
        std::map<std::string, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

        nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "intro_block";
        message_j["latest_block_nr"] = my_latest_block_nr;
        message_j["block"] = block_j;
        message_j["full_hash_coord"] = my_full_hash;

        std::string k, v;
        for (auto &[k, v] : parts)
        {
            message_j["chosen_ones"].push_back(k);
        }

        to_sign_j["latest_block_nr"] = my_latest_block_nr;
        to_sign_j["block"] = block_j;
        to_sign_j["full_hash_coord"] = my_full_hash;
        to_sign_j["chosen_ones"] = message_j["chosen_ones"];
        std::string to_sign_s = to_sign_j.dump();
        ECDSA<ECP, SHA256>::PrivateKey private_key;
        std::string signature;
        Crypto* crypto = new Crypto();
        crypto->ecdsa_load_private_key_from_string(private_key);
        if (crypto->ecdsa_sign_message(private_key, to_sign_s, signature))
        {
            message_j["signature"] = crypto->base64_encode(signature);
        }
        delete crypto;
        
        std::string srv_ip = ""; // only for nat traversal
        std::string peer_hash = ""; // dunno, still dunno

        Tcp tcp;
        std::string key, val;
        for (auto &[key, val] : parts)
        {
            if (key == my_full_hash) continue;

            Rocksy* rocksy = new Rocksy();

            std::string peer_id = rocksy->FindChosenOne(val); // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(peer_id));
            std::string peer_ip = value_j["ip"];

            delete rocksy;
            
            std::string message = message_j.dump();

            // tcp.client() to all chosen ones with intro_peer request
            tcp.client(srv_ip, peer_ip, peer_hash, message);
        }
    }
    else
    {
        // You're not the coordinator!
        std::cout << "You're not the coordinator!" << std::endl;
    }
}

// the block still needs to be hashed and the hash sent
// at receiver of intro_block side the block needs to be compared with the list of new_peers the receiver has
// we need to verify what decides the coordinator role, the hash is based on the txs or the complete block?
// who decides you're the coordinator? verify again!