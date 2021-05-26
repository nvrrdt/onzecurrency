// The plan to create coin:
// - wished call for a payment: onze1blahblah 1.01 ((to peer) (value in onze)
// - 1. create rudimentary input for this call
// - 2. intro_tx and new_tx, the chose_ones verify the funds of the payer
//      - send signed intro_tx to coordinator
//      - coordinator looks up public_key in blockchain and verifies
//      - if funds (in blockchain: base and payment are shown) are sufficient then send to chosen_ones and payer
//      - payer verifies a random chosen_one if payment is ok
//      - if verified and ok for chosen_ones then they send a new_tx to all
//      - if payer and payee receive a new_tx, then they must be assured payment is ok
// - 3. intro_block_c and new_block_c, the chosen_ones are rewarded an onze
// - headless state handling

#include "p2p_network_c.hpp"

#include "rocksy.hpp"
#include "json.hpp"
#include "full_hash.hpp"
#include "prev_hash.hpp"

using namespace Coin;

void P2pNetworkC::handle_read_server_c(nlohmann::json buf_j)
{
    //
    std::cout << "buf_j server " << buf_j << std::endl;

    std::string req = buf_j["req"];
    std::map<std::string, int> req_conversion;
    req_conversion["intro_tx"] =    20;
    req_conversion["new_tx"] =      21;

    switch (req_conversion[req])
    {
        case 20:    intro_tx(buf_j);
                    break;
        case 21:    new_tx(buf_j);
                    break;
    }
}

void P2pNetworkC::intro_tx(nlohmann::json buf_j)
{
    //
    std::string full_hash_req = buf_j["full_hash_req"];
    std::string to_full_hash = buf_j["tx_to"];
    std::string amount = buf_j["amount"];
    std::string signature = buf_j["signature"];

    Rocksy* rocksy = new Rocksy();
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(full_hash_req));
    if (contents_j == "")
    {
        std::cout << "Requester not in database" << std::endl;
        return;
    }
    std::string ecdsa_pub_key_s = contents_j["ecdsa_pub_key"];
    delete rocksy;

    nlohmann::json to_verify_j;
    to_verify_j["req"] = "intro_tx";
    to_verify_j["full_hash_req"] = full_hash_req;
    to_verify_j["tx_to"] = to_full_hash;
    to_verify_j["amount"] = amount;

    std::string to_verify_s = to_verify_j.dump();
    ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
    Crypto* crypto = new Crypto();
    crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
    std::string signature_bin = crypto->base64_decode(signature);
    
    if (crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
    {
        std::cout << "intro_tx: verified" << std::endl;

        FullHash fh;
        std::string my_full_hash = fh.get_full_hash_from_file();
        PrevHash ph;
        std::string hash_latest_block = ph.calculate_hash_from_last_block();
        std::string coordinator = my_full_hash + hash_latest_block;
        
        if (my_full_hash == coordinator)
        {
            std::cout << "intro_tx: I'm the coordinator" << std::endl;

            // Are funds sufficient? Create a second rocksdb here!
        }
        else
        {
            std::cout << "intro_tx: I'm not the coordinator" << std::endl;
        }
    }
    else
    {
        std::cout << "intro_tx: verification didn't succeed" << std::endl;
    }

    delete crypto;
}

void P2pNetworkC::new_tx(nlohmann::json buf_j)
{
    //
}