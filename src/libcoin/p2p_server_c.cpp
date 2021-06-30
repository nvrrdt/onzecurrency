// The plan to create coin:
// - wished call for a payment: onze1blahblah 1.01 ((to peer) (value in onze)
// - 1. create rudimentary input for this call
// - 2. new_tx, intro_tx and new_tx, the chose_ones verify the funds of the payer
//      - send signed new_tx to coordinator
//      - coordinator looks up public_key in blockchain and verifies
//      - if funds (in blockchain: base and payment are shown) are sufficient then send to chosen_ones// and payer
//      //OPTIONAL or LATER- payer polls a random chosen_one if payment is ok
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
#include "poco_coin.hpp"
#include "block_matrix.hpp"
#include "merkle_tree_c.hpp"

using namespace Common;
using namespace Coin;
using namespace Poco;

void P2pNetworkC::handle_read_server_c(nlohmann::json buf_j)
{
    //
    std::cout << "buf_j server " << buf_j << std::endl;

    std::string req = buf_j["req"];
    std::map<std::string, int> req_conversion;
    req_conversion["hello_tx"] =        20;
    req_conversion["intro_tx"] =        21;
    req_conversion["new_tx"] =          22;
    req_conversion["hello_reward"] =    23;
    req_conversion["intro_reward"] =    24;
    req_conversion["new_reward"] =      25;
    req_conversion["intro_block_c"] =   26;
    req_conversion["hash_comparison_c"] =   27;
    req_conversion["new_block_c"] =     28;

    switch (req_conversion[req])
    {
        case 20:    hello_tx(buf_j);
                    break;
        case 21:    intro_tx(buf_j);
                    break;
        case 22:    new_tx(buf_j);
                    break;
        case 23:    hello_reward(buf_j);
                    break;
        case 24:    intro_reward(buf_j);
                    break;
        case 25:    new_reward(buf_j);
                    break;
        case 26:    intro_block_c(buf_j);
                    break;
        case 27:    hash_comparison_c(buf_j);
                    break;
        case 28:    new_block_c(buf_j);
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
        std::string prel_coordinator = full_hash_req + hash_latest_block;

        Rocksy* rocksy = new Rocksy("usersdb");
        std::string full_hash_coordinator = rocksy->FindChosenOne(prel_coordinator);
        delete rocksy;
        
        if (my_full_hash == full_hash_coordinator && my_full_hash != full_hash_req)
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

            // calculate dev_amount
            Transactions txs;
            uint64_t dev_amount_number = txs.calculate_dev_payment_numbers(amount_number);

            // calculate total_amount
            uint64_t total_amount = amount_number + dev_amount_number;

            if (funds >= total_amount)
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

                PocoCoin poco;
                // The first part of the capstone implementation of poco:
                poco.evaluate_transactions();

                start_block_creation_thread();
            }
            else
            {
                std::cout << "Hello_tx: funds don't suffice" << std::endl;
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
    std::cout << "Intro_tx:" << std::endl;

    std::string full_hash_req = buf_j["full_hash_req"];
    std::string to_full_hash = buf_j["tx_to"];
    std::string amount = buf_j["amount"];
    std::string signature = buf_j["signature"];
    nlohmann::json chosen_ones = buf_j["chosen_ones"];

    // introduce chosen ones!!! and coordinator should be verified

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
    to_verify_j["req"] = "intro_tx";
    to_verify_j["full_hash_req"] = full_hash_req;
    to_verify_j["tx_to"] = to_full_hash;
    to_verify_j["amount"] = amount;
    to_verify_j["chosen_ones"] = chosen_ones;

    std::string to_verify_s = to_verify_j.dump();
    ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
    Crypto* crypto = new Crypto();
    crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
    std::string signature_bin = crypto->base64_decode(signature);
    
    if (crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
    {
        std::cout << "Intro_tx: verified" << std::endl;

        FullHash fh;
        std::string my_full_hash = fh.get_full_hash_from_file();

        PrevHash ph;
        std::string hash_latest_block = ph.calculate_hash_from_last_block();
        std::string coordinator = full_hash_req + hash_latest_block;

        bool is_chosen_one = false;
        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (chosen_ones[i] == my_full_hash) is_chosen_one = true;
        }

        if (is_chosen_one && my_full_hash != full_hash_req)
        {
            std::cout << "Intro_tx: I'm a chosen_one" << std::endl;

            // Are funds sufficient?
            Rocksy* rocksy = new Rocksy("transactionsdb");
            nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(full_hash_req));
            uint64_t funds = contents_j["funds"];
            delete rocksy;

            uint64_t amount_number;
            std::istringstream iss(amount);
            iss >> amount_number;

            // calculate dev_amount
            Transactions txs;
            uint64_t dev_amount_number = txs.calculate_dev_payment_numbers(amount_number);

            // calculate total_amount
            uint64_t total_amount = amount_number + dev_amount_number;

            if (funds >= total_amount)
            {
                std::cout << "funds are ok" << std::endl;

                // Inform network below here!
                Protocol proto;
                std::map<int, std::string> parts;

                for (int i = 0; i < buf_j["chosen_ones"].size(); i++)
                {
                    if (buf_j["chosen_ones"][i] == my_full_hash && i != buf_j["chosen_ones"].size() - 1)
                    {
                        std::string next_full_hash = buf_j["chosen_ones"][i+1];
                        parts = proto.partition_in_buckets(my_full_hash, next_full_hash);
                        break;
                    }
                    else if (buf_j["chosen_ones"][i] == my_full_hash && i == buf_j["chosen_ones"].size() - 1)
                    {
                        std::string next_full_hash = buf_j["chosen_ones"][0];
                        parts = proto.partition_in_buckets(my_full_hash, next_full_hash);
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }

                nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
                message_j["req"] = "new_tx";
                message_j["full_hash_req"] = full_hash_req;
                message_j["tx_to"] = to_full_hash;
                message_j["amount"] = amount;

                int k;
                std::string v;
                for (auto &[k, v] : parts)
                {
                    message_j["chosen_ones"].push_back(v);
                }

                to_sign_j["req"] = "new_tx";
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

                    std::cout << "Preparation for new_tx: " << peer_ip << std::endl;

                    std::string ip_from_peer;
                    P2p p2p;
                    p2p.number_to_ip_string(peer_ip, ip_from_peer);

                    // p2p_client() to all chosen ones with intro_tx request
                    pn.p2p_client(ip_from_peer, message);
                }

                // Save the tx here in a static variable
                Transactions tx;
                tx.add_tx_to_transactions(full_hash_req, to_full_hash, amount);

                PocoCoin poco;
                // The first part of the capstone implementation of poco:
                poco.evaluate_transactions();

                start_block_creation_thread();
            }
            else
            {
                std::cout << "Intro_tx: funds don't suffice" << std::endl;
            }
        }
        else
        {
            std::cout << "Intro_tx: I'm not chosen_one, maybe try again in a few minutes" << std::endl;
        }
    }
    else
    {
        std::cout << "Intro_tx: verification didn't succeed" << std::endl;
    }

    delete crypto;
}

void P2pNetworkC::new_tx(nlohmann::json buf_j)
{
    //
    std::cout << "New_tx:" << std::endl;

    std::string full_hash_req = buf_j["full_hash_req"];
    std::string to_full_hash = buf_j["tx_to"];
    std::string amount = buf_j["amount"];
    std::string signature = buf_j["signature"];
    nlohmann::json chosen_ones = buf_j["chosen_ones"];

    // introduce chosen ones!!! and coordinator should be verified

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
    to_verify_j["req"] = "new_tx";
    to_verify_j["full_hash_req"] = full_hash_req;
    to_verify_j["tx_to"] = to_full_hash;
    to_verify_j["amount"] = amount;
    to_verify_j["chosen_ones"] = chosen_ones;

    std::string to_verify_s = to_verify_j.dump();
    ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
    Crypto* crypto = new Crypto();
    crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
    std::string signature_bin = crypto->base64_decode(signature);
    
    if (crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
    {
        std::cout << "New_tx: verified" << std::endl;

        FullHash fh;
        std::string my_full_hash = fh.get_full_hash_from_file();

        PrevHash ph;
        std::string hash_latest_block = ph.calculate_hash_from_last_block();
        std::string coordinator = full_hash_req + hash_latest_block;

        bool is_chosen_one = false;
        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (chosen_ones[i] == my_full_hash) is_chosen_one = true;
        }

        if (is_chosen_one && my_full_hash != full_hash_req)
        {
            std::cout << "New_tx: I'm a secondary chosen_one" << std::endl;

            // Are funds sufficient?
            Rocksy* rocksy = new Rocksy("transactionsdb");
            nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(full_hash_req));
            uint64_t funds = contents_j["funds"];
            delete rocksy;

            uint64_t amount_number;
            std::istringstream iss(amount);
            iss >> amount_number;

            // calculate dev_amount
            Transactions txs;
            uint64_t dev_amount_number = txs.calculate_dev_payment_numbers(amount_number);

            // calculate total_amount
            uint64_t total_amount = amount_number + dev_amount_number;

            if (funds >= total_amount)
            {
                std::cout << "funds are ok" << std::endl;

                // Inform network below here!
                Protocol proto;
                std::map<int, std::string> parts;

                for (int i = 0; i < buf_j["chosen_ones"].size(); i++)
                {
                    if (buf_j["chosen_ones"][i] == my_full_hash && i != buf_j["chosen_ones"].size() - 1)
                    {
                        std::string next_full_hash = buf_j["chosen_ones"][i+1];
                        parts = proto.partition_in_buckets(my_full_hash, next_full_hash);
                        break;
                    }
                    else if (buf_j["chosen_ones"][i] == my_full_hash && i == buf_j["chosen_ones"].size() - 1)
                    {
                        std::string next_full_hash = buf_j["chosen_ones"][0];
                        parts = proto.partition_in_buckets(my_full_hash, next_full_hash);
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }

                nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
                message_j["req"] = "new_tx";
                message_j["full_hash_req"] = full_hash_req;
                message_j["tx_to"] = to_full_hash;
                message_j["amount"] = amount;

                int k;
                std::string v;
                for (auto &[k, v] : parts)
                {
                    message_j["chosen_ones"].push_back(v);
                }

                to_sign_j["req"] = "new_tx";
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

                    std::cout << "Preparation for new_tx: " << peer_ip << std::endl;

                    std::string ip_from_peer;
                    P2p p2p;
                    p2p.number_to_ip_string(peer_ip, ip_from_peer);

                    // p2p_client() to all chosen ones with new_tx request
                    pn.p2p_client(ip_from_peer, message);
                }

                // Save the tx here in a static variable
                Transactions tx;
                tx.add_tx_to_transactions(full_hash_req, to_full_hash, amount);

                PocoCoin poco;
                // The first part of the capstone implementation of poco:
                poco.evaluate_transactions();

                start_block_creation_thread();
            }
            else
            {
                std::cout << "New_tx: funds don't suffice" << std::endl;
            }
        }
        else
        {
            std::cout << "New_tx: I'm not a secondary chosen_one, maybe try again in a few minutes" << std::endl;
        }
    }
    else
    {
        std::cout << "New_tx: verification didn't succeed" << std::endl;
    }

    delete crypto;
}

void P2pNetworkC::hello_reward(nlohmann::json buf_j)
{
    //
    std::cout << "Hello_reward:" << std::endl;

    std::string full_hash_req = buf_j["full_hash_req"];
    std::string hash_of_block = buf_j["hash_of_block"];
    nlohmann::json chosen_ones_reward = buf_j["chosen_ones_reward"];
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
    to_verify_j["req"] = "hello_reward";
    to_verify_j["full_hash_req"] = full_hash_req;
    to_verify_j["hash_of_block"] = hash_of_block;
    to_verify_j["chosen_ones_reward"] = chosen_ones_reward;

    std::string to_verify_s = to_verify_j.dump();
    ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
    Crypto* crypto = new Crypto();
    crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
    std::string signature_bin = crypto->base64_decode(signature);
    
    if (crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
    {
        std::cout << "Hello_reward: verified" << std::endl;

        FullHash fh;
        std::string my_full_hash = fh.get_full_hash_from_file();

        Rocksy* rocksy = new Rocksy("usersdb");
        std::string chosen_ones_s = chosen_ones_reward.dump();
        std::string hash_of_cos = crypto->bech32_encode_sha256(chosen_ones_s);
        std::string coordinator = rocksy->FindChosenOne(hash_of_cos);
        delete rocksy;
        
        if (my_full_hash == coordinator)
        {
            std::cout << "Hello_reward: I'm the coordinator" << std::endl;

            // Inform chosen_ones and add 1 onze to my fund

            Protocol proto;
            std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

            nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
            message_j["req"] = "intro_reward";
            message_j["hash_of_block"] = hash_of_block;
            message_j["chosen_ones_reward"] = chosen_ones_reward;

            int k;
            std::string v;
            for (auto &[k, v] : parts)
            {
                message_j["chosen_ones"].push_back(v);
            }

            to_sign_j["req"] = "intro_reward";
            to_sign_j["hash_of_block"] = hash_of_block;
            to_sign_j["chosen_ones_reward"] = chosen_ones_reward;
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
                // if (val == full_hash_req) continue; // TODO: Maybe a chosen_one_reward shouldn't be a chosen_one
                if (val == my_full_hash || val == "" || val == "0") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
                
                Rocksy* rocksy = new Rocksy("usersdb");

                // lookup in rocksdb
                nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
                uint32_t peer_ip = value_j["ip"];

                delete rocksy;
                
                std::string message = message_j.dump();

                std::cout << "Preparation for intro_reward: " << peer_ip << std::endl;

                std::string ip_from_peer;
                P2p p2p;
                p2p.number_to_ip_string(peer_ip, ip_from_peer);

                // p2p_client() to all chosen ones with intro_reward request
                pn.p2p_client(ip_from_peer, message);
            }

            // Save the txs here in a static variable
            Transactions *tx = new Transactions();
            std::string amount = "1"; // 1 onze
            for (int i = 0; i < chosen_ones_reward.size(); i++)
            {
                std::string to_full_hash = chosen_ones_reward[i];

                if (to_full_hash == "0")
                {
                    break;
                }
                else
                {
                    tx->add_tx_to_transactions(coordinator, to_full_hash, amount);
                }

                if (i == 0)
                {
                    tx->set_new_rewards(true);
                    start_block_creation_thread();
                    tx->set_new_rewards(false);
                }
            }

            delete tx;
        }
        else
        {
            std::cout << "Hello_reward: I'm not the coordinator, try again in a few minutes" << std::endl;
        }
    }
    else
    {
        std::cout << "Hello_reward: verification didn't succeed" << std::endl;
    }

    delete crypto;
}

void P2pNetworkC::intro_reward(nlohmann::json buf_j)
{
    //
    std::cout << "Intro_reward:" << std::endl;

    std::string hash_of_block = buf_j["hash_of_block"];
    nlohmann::json chosen_ones_reward = buf_j["chosen_ones_reward"];
    std::string signature = buf_j["signature"];
    nlohmann::json chosen_ones = buf_j["chosen_ones"];

    // introduce chosen ones!!! and coordinator should be verified

    Rocksy* rocksy = new Rocksy("usersdb");
    std::string coordinator = chosen_ones[0];
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(coordinator));
    if (contents_j == "")
    {
        std::cout << "Requester not in database" << std::endl;
        return;
    }
    std::string ecdsa_pub_key_s = contents_j["ecdsa_pub_key"];
    delete rocksy;

    nlohmann::json to_verify_j;
    to_verify_j["req"] = "intro_reward";
    to_verify_j["hash_of_block"] = hash_of_block;
    to_verify_j["chosen_ones_reward"] = chosen_ones_reward;
    to_verify_j["chosen_ones"] = chosen_ones;

    std::string to_verify_s = to_verify_j.dump();
    ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
    Crypto* crypto = new Crypto();
    crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
    std::string signature_bin = crypto->base64_decode(signature);
    
    if (crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
    {
        std::cout << "Intro_reward: verified" << std::endl;

        FullHash fh;
        std::string my_full_hash = fh.get_full_hash_from_file();

        bool is_chosen_one = false;
        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (chosen_ones[i] == my_full_hash) is_chosen_one = true;
        }
        
        if (is_chosen_one)
        {
            std::cout << "Intro_reward: I'm a chosen_one" << std::endl;

            // Inform network below here!
            Protocol proto;
            std::map<int, std::string> parts;

            for (int i = 0; i < buf_j["chosen_ones"].size(); i++)
            {
                if (buf_j["chosen_ones"][i] == my_full_hash && i != buf_j["chosen_ones"].size() - 1)
                {
                    std::string next_full_hash = buf_j["chosen_ones"][i+1];
                    parts = proto.partition_in_buckets(my_full_hash, next_full_hash);
                    break;
                }
                else if (buf_j["chosen_ones"][i] == my_full_hash && i == buf_j["chosen_ones"].size() - 1)
                {
                    std::string next_full_hash = buf_j["chosen_ones"][0];
                    parts = proto.partition_in_buckets(my_full_hash, next_full_hash);
                    break;
                }
                else
                {
                    continue;
                }
            }

            nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
            message_j["req"] = "new_reward";
            message_j["hash_of_block"] = hash_of_block;
            message_j["chosen_ones_reward"] = chosen_ones_reward;

            int k;
            std::string v;
            for (auto &[k, v] : parts)
            {
                message_j["chosen_ones"].push_back(v);
            }

            to_sign_j["req"] = "new_reward";
            to_sign_j["hash_of_block"] = hash_of_block;
            to_sign_j["chosen_ones_reward"] = chosen_ones_reward;
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
                // if (val == full_hash_req) continue; // TODO: Maybe a chosen_one_reward shouldn't be a chosen_one
                if (val == my_full_hash || val == "" || val == "0") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
                
                Rocksy* rocksy = new Rocksy("usersdb");

                // lookup in rocksdb
                nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
                uint32_t peer_ip = value_j["ip"];

                delete rocksy;
                
                std::string message = message_j.dump();

                std::cout << "Preparation for new_reward: " << peer_ip << std::endl;

                std::string ip_from_peer;
                P2p p2p;
                p2p.number_to_ip_string(peer_ip, ip_from_peer);

                // p2p_client() to all chosen ones with intro_reward request
                pn.p2p_client(ip_from_peer, message);
            }

            // Save the txs here in a static variable
            Transactions *tx = new Transactions();
            std::string full_hash_req = coordinator;
            std::string amount = "1"; // 1 onze
            for (int i = 0; i < chosen_ones_reward.size(); i++)
            {
                std::string to_full_hash = chosen_ones_reward[i];

                if (to_full_hash == "0")
                {
                    break;
                }
                else
                {
                    tx->add_tx_to_transactions(full_hash_req, to_full_hash, amount);
                }

                if (i == 0)
                {
                    tx->set_new_rewards(true);
                    start_block_creation_thread();
                    tx->set_new_rewards(false);
                }
            }

            delete tx;
        }
        else
        {
            std::cout << "Intro_reward: I'm not chosen_one, maybe try again in a few minutes" << std::endl;
        }
    }
    else
    {
        std::cout << "Intro_reward: verification didn't succeed" << std::endl;
    }

    delete crypto;
}

void P2pNetworkC::new_reward(nlohmann::json buf_j)
{
    //
    std::cout << "New_reward:" << std::endl;

    std::string hash_of_block = buf_j["hash_of_block"];
    nlohmann::json chosen_ones_reward = buf_j["chosen_ones_reward"];
    std::string signature = buf_j["signature"];
    nlohmann::json chosen_ones = buf_j["chosen_ones"];

    // introduce chosen ones!!! and coordinator should be verified

    Rocksy* rocksy = new Rocksy("usersdb");
    std::string coordinator = chosen_ones[0];
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(coordinator));
    if (contents_j == "")
    {
        std::cout << "Requester not in database" << std::endl;
        return;
    }
    std::string ecdsa_pub_key_s = contents_j["ecdsa_pub_key"];
    delete rocksy;

    nlohmann::json to_verify_j;
    to_verify_j["req"] = "new_reward";
    to_verify_j["hash_of_block"] = hash_of_block;
    to_verify_j["chosen_ones_reward"] = chosen_ones_reward;;
    to_verify_j["chosen_ones"] = chosen_ones;

    std::string to_verify_s = to_verify_j.dump();
    ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
    Crypto* crypto = new Crypto();
    crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
    std::string signature_bin = crypto->base64_decode(signature);
    
    if (crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
    {
        std::cout << "New_reward: verified" << std::endl;

        FullHash fh;
        std::string my_full_hash = fh.get_full_hash_from_file();

        bool is_chosen_one = false;
        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (chosen_ones[i] == my_full_hash) is_chosen_one = true;
        }
        
        if (is_chosen_one)
        {
            std::cout << "Intro_reward: I'm a secondary chosen_one" << std::endl;

            // Inform network below here!
            Protocol proto;
            std::map<int, std::string> parts;

            for (int i = 0; i < buf_j["chosen_ones"].size(); i++)
            {
                if (buf_j["chosen_ones"][i] == my_full_hash && i != buf_j["chosen_ones"].size() - 1)
                {
                    std::string next_full_hash = buf_j["chosen_ones"][i+1];
                    parts = proto.partition_in_buckets(my_full_hash, next_full_hash);
                    break;
                }
                else if (buf_j["chosen_ones"][i] == my_full_hash && i == buf_j["chosen_ones"].size() - 1)
                {
                    std::string next_full_hash = buf_j["chosen_ones"][0];
                    parts = proto.partition_in_buckets(my_full_hash, next_full_hash);
                    break;
                }
                else
                {
                    continue;
                }
            }

            nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
            message_j["req"] = "new_reward";
            message_j["hash_of_block"] = hash_of_block;
            message_j["chosen_ones_reward"] = chosen_ones_reward;

            int k;
            std::string v;
            for (auto &[k, v] : parts)
            {
                message_j["chosen_ones"].push_back(v);
            }

            to_sign_j["req"] = "new_reward";
            to_sign_j["hash_of_block"] = hash_of_block;
            to_sign_j["chosen_ones_reward"] = chosen_ones_reward;
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
                // if (val == full_hash_req) continue; // TODO: Maybe a chosen_one_reward shouldn't be a chosen_one
                if (val == my_full_hash || val == "" || val == "0") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
                
                Rocksy* rocksy = new Rocksy("usersdb");

                // lookup in rocksdb
                nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
                uint32_t peer_ip = value_j["ip"];

                delete rocksy;
                
                std::string message = message_j.dump();

                std::cout << "Preparation for new_reward: " << peer_ip << std::endl;

                std::string ip_from_peer;
                P2p p2p;
                p2p.number_to_ip_string(peer_ip, ip_from_peer);

                // p2p_client() to all chosen ones with intro_reward request
                pn.p2p_client(ip_from_peer, message);
            }

            // Save the txs here in a static variable
            Transactions *tx = new Transactions();
            std::string full_hash_req = coordinator;
            std::string amount = "1"; // 1 onze
            for (int i = 0; i < chosen_ones_reward.size(); i++)
            {
                std::string to_full_hash = chosen_ones_reward[i];

                if (to_full_hash == "0")
                {
                    break;
                }
                else
                {
                    tx->add_tx_to_transactions(full_hash_req, to_full_hash, amount);
                }

                if (i == 0)
                {
                    tx->set_new_rewards(true);
                    start_block_creation_thread();
                    tx->set_new_rewards(false);
                }
            }

            delete tx;
        }
        else
        {
            std::cout << "New_reward: I'm not a secondary chosen_one, maybe try again in a few minutes" << std::endl;
        }
    }
    else
    {
        std::cout << "New_reward: verification didn't succeed" << std::endl;
    }

    delete crypto;
}

void P2pNetworkC::start_block_creation_thread()
{
    Transactions *tx = new Transactions();

    if (tx->get_transactions().size() > 2048) // size limit of block within block creation delay
    {
        // Create block
        PocoCoin poco;
        poco.create_and_send_block_c();

        tx->reset_transactions();
    }
    else if (tx->get_transactions().size() == 1 || tx->get_new_rewards())
    {
        // wait 20 secs
        // then create block

        std::cout << "Get_sleep_and_create_block_c" << std::endl;

        std::thread t(&P2pNetworkC::get_sleep_and_create_block_server_c, this);
        t.detach();
    }

    delete tx;
}

void P2pNetworkC::get_sleep_and_create_block_server_c()
{
    std::this_thread::sleep_for(std::chrono::seconds(10));

    Transactions tx;
    std::cout << "transactions.size() in Coin: " << tx.get_transactions().size() << std::endl;

    BlockMatrix *bm = new BlockMatrix();
    bm->add_received_block_vector_to_received_block_matrix();

    PocoCoin poco;
    poco.create_and_send_block_c(); // chosen ones are being informed here

    std::cout << "Block_c created server!!" << std::endl;

    delete bm;
}

void P2pNetworkC::intro_block_c(nlohmann::json buf_j)
{
    std::cout << "Intro_block_c: " << std::endl;

    // Compare the hashes from the block of the coordinator with this block
    // p2p_client to other chosen_ones --> needs to be calculated depending on this server's place in the chosen_ones list
    // Communicate hash to all
    // Then inform your underlying network
    // Put block in waiting list until it's the only block in the chain --> future implementation with coin

    // Is the coordinator the truthful real coordinator for this block

    std::string full_hash_coord_from_coord = buf_j["full_hash_coord"];

    nlohmann::json starttime_coord = buf_j["block"]["starttime"];

    nlohmann::json recv_block_j = buf_j["block"];
    BlockMatrix *bm = new BlockMatrix();
    if (bm->get_block_matrix().empty())
    {
        std::cout << "Received block is in block_vector; 1" << std::endl;
        bm->add_received_block_to_received_block_vector(recv_block_j);
    }
    else
    {
        for (uint16_t i = 0; i < bm->get_block_matrix().back().size(); i++)
        {
            if (*bm->get_block_matrix().back().at(i) == recv_block_j)
            {
                std::cout << "Received block is in block_vector; 2" << std::endl;
                bm->add_received_block_to_received_block_vector(recv_block_j);
                break;
            }
            
            if (i == bm->get_block_matrix().back().size() - 1)
            {
                // Don't accept this block
                std::cout << "Received block not in block_vector" << std::endl;
                return;
            }
        }
    }

    delete bm;

    Common::Crypto crypto;

    recv_block_j["starttime"] = starttime_coord;
    std::string recv_block_s = recv_block_j.dump();
    std::string prev_hash_me = crypto.bech32_encode_sha256(recv_block_s);

    Rocksy* rocksy = new Rocksy("usersdb");
    std::string full_hash_coord_from_me = rocksy->FindChosenOne(prev_hash_me);
    if (full_hash_coord_from_me == buf_j["full_hash_req"])
    {
        full_hash_coord_from_me = rocksy->FindNextPeer(full_hash_coord_from_me);
    }
    delete rocksy;

    FullHash fh;
    std::string my_full_hash = fh.get_full_hash_from_file();

    if (full_hash_coord_from_me == my_full_hash)
    {
        std::cout << "Coordinator is truthful c" << std::endl;

        std::string prev_hash_coordinator = buf_j["prev_hash"];
        std::string prev_hash_in_block = buf_j["block"]["prev_hash"];

        if (prev_hash_coordinator == prev_hash_in_block)
        {
            std::cout << "Successful comparison of prev_hashes, now sharing hashes c" << std::endl;

            // Put in rocksdb
            // for (auto &[key, value] : buf_j["rocksdb"].items()) // TODO not yet ready in poco_c
            // {
            //     std::string key_s = value["full_hash"];
            //     std::string value_s = value.dump();

            //     Rocksy* rocksy = new Rocksy("usersdb");
            //     rocksy->Put(key_s, value_s);
            //     delete rocksy;
            // }
        }
        else
        {
            std::cout << "Unsuccessful comparison of prev_hashes c" << std::endl;
        }

        // Inform coordinator of succesfullness of hash comparison
        nlohmann::json m_j;
        m_j["req"] = "hash_comparison_c";
        m_j["hash_comp"] = prev_hash_in_block == prev_hash_coordinator;
        std::string msg_s = m_j.dump();

        P2pNetwork::set_resp_msg_server(msg_s);

        // p2p_client() to all calculated other chosen_ones
        // this is in fact the start of the consensus algorithm
        // you don't need full consensus in order to create a succesful block
        // but full consensus improves your chances of course greatly
        nlohmann::json chosen_ones = buf_j["chosen_ones"];

        FullHash fh;
        std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be
        // std::cout << "My_full_hash already present in file: " << my_full_hash << std::endl;

        int j;

        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (chosen_ones[i] == buf_j["full_hash_req"])
            {
                j = i;
            }
        }

        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (i < j)
            {
                if (chosen_ones[i] == buf_j["full_hash_coord"]) continue;

                std::string c_one = chosen_ones[i];
                Rocksy* rocksy = new Rocksy("usersdb");
                nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(c_one));
                delete rocksy;

                enet_uint32 ip = value_j["ip"];
                std::string peer_ip;
                P2p p2p;
                p2p.number_to_ip_string(ip, peer_ip);

                nlohmann::json msg_j;
                msg_j["req"] = "hash_comparison_c";
                msg_j["hash_comp"] = prev_hash_me == prev_hash_coordinator;
                std::string msg_s = msg_j.dump();

                p2p_client(peer_ip, msg_s);
            }
            else if (i == j)
            {
                continue;
            }
        }
    }
    else
    {
        std::cout << "Coordinator is not truthful c" << std::endl;
    }

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());
}

void P2pNetworkC::hash_comparison_c(nlohmann::json buf_j)
{
    // compare the received hash
    std::cout << "The hash comparison is (server): " <<  buf_j["hash_comp"] << std::endl;
}

void P2pNetworkC::new_block_c(nlohmann::json buf_j)
{
    // new_block --> TODO block and rocksdb should be saved
    std::cout << "New_block_c: " << std::endl;
}