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
#include "transactions.hpp"

using namespace Coin;

void P2pNetworkC::handle_read_server_c(nlohmann::json buf_j)
{
    //
    std::cout << "buf_j server " << buf_j << std::endl;

    std::string req = buf_j["req"];
    std::map<std::string, int> req_conversion;
    req_conversion["hello_tx"] =        20;
    req_conversion["intro_tx"] =        21;
    req_conversion["new_tx"] =          22;
    req_conversion["intro_block_c"] =   23;
    req_conversion["new_block_c"] =     24;

    switch (req_conversion[req])
    {
        case 20:    hello_tx(buf_j);
                    break;
        case 21:    intro_tx(buf_j);
                    break;
        case 22:    new_tx(buf_j);
                    break;
        case 23:    intro_block_c(buf_j);
                    break;
        case 24:    new_block_c(buf_j);
                    break;
    }
}

void P2pNetworkC::hello_tx(nlohmann::json buf_j)
{
    //
    std::cout << "Hello_tx:" << std::endl;

    std::string full_hash_req = buf_j["full_hash_req"];
    std::string to_full_hash = buf_j["tx_to"];
    std::string amount = buf_j["amount"];
    std::string signature = buf_j["signature"];

    Rocksy* rocksy = new Rocksy("usersdb");
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(full_hash_req));
    if (contents_j == "")
    {
        std::cout << "Requester not in database" << std::endl;
        return;
    }
    std::string ecdsa_pub_key_s = contents_j["ecdsa_pub_key"];
    delete rocksy;

    nlohmann::json to_verify_j;
    to_verify_j["req"] = "hello_tx";
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
        std::cout << "Hello_tx: verified" << std::endl;

        FullHash fh;
        std::string my_full_hash = fh.get_full_hash_from_file();

        PrevHash ph;
        std::string hash_latest_block = ph.calculate_hash_from_last_block();
        std::string coordinator = full_hash_req + hash_latest_block;
        
        if (my_full_hash == coordinator && my_full_hash != full_hash_req)
        {
            std::cout << "Hello_tx: I'm the coordinator" << std::endl;

            // Are funds sufficient? Create a second rocksdb here!
            Rocksy* rocksy = new Rocksy("transactionsdb");
            nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(full_hash_req));
            uint64_t funds = contents_j["funds"];
            delete rocksy;

            uint64_t amount_number;
            std::istringstream iss(amount);
            iss >> amount_number;

            if (funds >= amount_number)
            {
                std::cout << "funds are ok" << std::endl;

                // Inform chosen_ones here!
                Protocol proto;
                std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

                nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
                message_j["req"] = "intro_tx";
                message_j["full_hash_req"] = full_hash_req;
                message_j["tx_to"] = to_full_hash;
                message_j["amount"] = amount;

                int k;
                std::string v;
                for (auto &[k, v] : parts)
                {
                    message_j["chosen_ones"].push_back(v);
                }

                to_sign_j["req"] = "intro_tx";
                to_sign_j["full_hash_req"] = full_hash_req;
                to_sign_j["tx_to"] = to_full_hash;
                to_sign_j["amount"] = amount;
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

                P2pNetwork pn;
                std::string key, val;
                for (auto &[key, val] : parts)
                {
                    if (key == 1) continue;
                    if (val == full_hash_req) continue;
                    if (val == my_full_hash || val == "" || val == "0") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
                    
                    Rocksy* rocksy = new Rocksy("usersdb");

                    // lookup in rocksdb
                    nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
                    uint32_t peer_ip = value_j["ip"];

                    delete rocksy;
                    
                    std::string message = message_j.dump();

                    std::cout << "Preparation for intro_tx: " << peer_ip << std::endl;

                    std::string ip_from_peer;
                    P2p p2p;
                    p2p.number_to_ip_string(peer_ip, ip_from_peer);

                    // p2p_client() to all chosen ones with intro_tx request
                    pn.p2p_client(ip_from_peer, message);
                }

                // Save the tx here in a static variable
                Transactions tx;
                tx.add_tx_to_transactions(full_hash_req, to_full_hash, amount);
            }
            else
            {
                std::cout << "funds don't suffice" << std::endl;
            }
        }
        else
        {
            std::cout << "Hello_tx: I'm not the coordinator, try again in a few minutes" << std::endl;
        }
    }
    else
    {
        std::cout << "Hello_tx: verification didn't succeed" << std::endl;
    }

    delete crypto;
}

void P2pNetworkC::intro_tx(nlohmann::json buf_j)
{
    //
}

void P2pNetworkC::new_tx(nlohmann::json buf_j)
{
    //
}

void P2pNetworkC::intro_block_c(nlohmann::json buf_j)
{
    //
}

void P2pNetworkC::new_block_c(nlohmann::json buf_j)
{
    //
}