#include "poco.hpp"
#include "p2p.hpp"
#include "auth.hpp"
#include "crypto.hpp"
#include "p2p_network.hpp"
#include "merkle_tree.hpp"

using namespace Crowd;

void Poco::create_and_send_block()
{
    merkle_tree mt;

    nlohmann::json m_j, entry_tx_j, entry_transactions_j, exit_tx_j, exit_transactions_j;
    nlohmann::json to_block_j;
    std::string fh_s;
    std::string full_hash_req;

    for (int i = 0; i < message_j_vec_.get_message_j_vec().size(); i++)
    {
        m_j = *message_j_vec_.get_message_j_vec()[i];

        full_hash_req = m_j["full_hash_req"];

        to_block_j["full_hash"] = full_hash_req;
        to_block_j["ecdsa_pub_key"] = m_j["ecdsa_pub_key"];
        to_block_j["rsa_pub_key"] = m_j["rsa_pub_key"];
        s_shptr_->push(to_block_j.dump());

        entry_tx_j["full_hash"] = to_block_j["full_hash"];
        entry_tx_j["ecdsa_pub_key"] = to_block_j["ecdsa_pub_key"];
        entry_tx_j["rsa_pub_key"] = to_block_j["rsa_pub_key"];
        entry_transactions_j.push_back(entry_tx_j);
        exit_tx_j["full_hash"] = "";
        exit_transactions_j.push_back(exit_tx_j);
    }

    s_shptr_ = mt.calculate_root_hash(s_shptr_);
    std::string datetime = mt.time_now();
    std::string root_hash_data = s_shptr_->top();
    block_j_ = mt.create_block(datetime, root_hash_data, entry_transactions_j, exit_transactions_j);

    PrevHash ph;
    block_j_["prev_hash"] = ph.calculate_hash_from_last_block();

    Protocol proto;
    std::string my_last_block_nr = proto.get_last_block_nr();

    std::string my_next_block_nr;
    uint64_t value;
    std::istringstream iss(my_last_block_nr);
    iss >> value;
    value++;
    std::ostringstream oss;
    oss << value;
    my_next_block_nr = oss.str();

    // send hash of this block with the block contents to the co's, forget save_block_to_file
    // is the merkle tree sorted, then find the last blocks that are gathered for all the co's

    // send intro_block to co's
    inform_chosen_ones(my_next_block_nr, block_j_, full_hash_req);

    message_j_vec_.reset_message_j_vec();
    all_hashes_.reset_all_hashes();

std::cout << "--------5: " << std::endl;
}

void Poco::inform_chosen_ones(std::string my_next_block_nr, nlohmann::json block_j, std::string full_hash_req)
{
    FullHash fh;
    std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be
    // std::cout << "My_full_hash already present in file:__ " << my_full_hash << std::endl;

    Crypto* crypto = new Crypto();
    std::string block_s = block_j.dump();
    std::string hash_of_block = crypto->bech32_encode_sha256(block_s);
    delete crypto;
    Rocksy* rocksy = new Rocksy("usersdb");
    std::string co_from_this_block = rocksy->FindChosenOne(hash_of_block);
    delete rocksy;

    nlohmann::json message_j;

    if (co_from_this_block == my_full_hash)
    {
        // You are the coordinator!
        std::cout << "Inform my fellow chosen_ones as coordinator" << std::endl;

        Protocol proto;
        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

        nlohmann::json to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "intro_block";
        message_j["latest_block_nr"] = my_next_block_nr;
        message_j["block"] = block_j;
        PrevHash ph;
        message_j["prev_hash"] = ph.calculate_hash_from_last_block();
        message_j["full_hash_req"] = full_hash_req;
        message_j["full_hash_coord"] = hash_of_block;

        int k;
        std::string v;
        for (auto &[k, v] : parts)
        {
            message_j["chosen_ones"].push_back(v);
        }

        // this for loop should be in inform_chosen_ones coordinator and send with an intro_block and a new_block
        // intro_block and new_block should put this in rocksdb
        // also a your_full_hash should receive and store this
        // and then the coordinator should put in rocksdb
        for (int i = 0; i < message_j_vec_.get_message_j_vec().size(); i++)
        {
            nlohmann::json m_j;
            m_j = *message_j_vec_.get_message_j_vec()[i];

            std::string full_hash_req = m_j["full_hash_req"];
            
            Crypto crypto;
            // update rocksdb
            nlohmann::json rocksdb_j;
            rocksdb_j["version"] = "O.1";
            rocksdb_j["ip"] = m_j["ip"];
            rocksdb_j["online"] = true;
            rocksdb_j["server"] = true;
            rocksdb_j["fullnode"] = true;
            std::string email_of_req = m_j["email_of_req"];
            rocksdb_j["hash_email"] = crypto.bech32_encode_sha256(email_of_req);
            rocksdb_j["prev_hash"] = ph.calculate_hash_from_last_block();
            rocksdb_j["full_hash"] = full_hash_req;
            Protocol proto;
            rocksdb_j["block_nr"] = proto.get_last_block_nr();
            rocksdb_j["ecdsa_pub_key"] = m_j["ecdsa_pub_key"];
            rocksdb_j["rsa_pub_key"] = m_j["rsa_pub_key"];
            std::string rocksdb_s = rocksdb_j.dump();

            // Store to rocksdb for coordinator
            Rocksy* rocksy = new Rocksy("usersdb");
            rocksy->Put(full_hash_req, rocksdb_s);
            delete rocksy;

            // Send rocksdb to into_block chosen_ones
            message_j["rocksdb"][i] = rocksdb_j;
        }

        to_sign_j["latest_block_nr"] = my_next_block_nr;
        to_sign_j["block"] = block_j;
        to_sign_j["prev_hash"] = ph.calculate_hash_from_last_block();
        to_sign_j["full_hash_req"] = full_hash_req;
        to_sign_j["full_hash_coord"] = hash_of_block;
        to_sign_j["chosen_ones"] = message_j["chosen_ones"];
        to_sign_j["rocksdb"] = message_j["rocksdb"];
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

            std::cout << "Preparation for intro_block: " << peer_ip << std::endl;

            std::string ip_from_peer;
            P2p p2p;
            p2p.number_to_ip_string(peer_ip, ip_from_peer);

            // p2p_client() to all chosen ones with intro_peer request
            pn.p2p_client(ip_from_peer, message);
        }

        merkle_tree mt;
        std::string block_s = mt.save_block_to_file(block_j, my_next_block_nr);

        set_hash_of_new_block(block_s);

        // Send your_full_hash request to intro_peer's
        for (auto &[key, value] : all_hashes_.get_all_hashes())
        {
            nlohmann::json msg_j;
            msg_j["req"] = "your_full_hash";
            std::vector<std::string> vec = *value;
            msg_j["full_hash"] = vec[0];
            msg_j["prev_hash"] = vec[1];
            msg_j["block"] = block_j;
            Protocol proto;
            msg_j["block_nr"] = proto.get_last_block_nr();

            msg_j["rocksdb"] = message_j["rocksdb"];

            std::string msg_s = msg_j.dump();

            std::string peer_ip;
            P2p p2p;
            p2p.number_to_ip_string(key, peer_ip);
            
            std::cout << "_______key: " << key << " ip: " << peer_ip << ", value: " << value << std::endl;
            P2pNetwork pn;
            pn.p2p_client(peer_ip, msg_s);
        }

        // Give the chosen_ones their reward:
        nlohmann::json chosen_ones_reward = message_j["chosen_ones"];
        reward_for_chosen_ones(co_from_this_block, chosen_ones_reward);
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

nlohmann::json Poco::get_block_j()
{
    // TODO intro_peer doesn't know its full_hash until new_block is send
    // but the other users don't know the whereabouts of intro_peer

    return block_j_;
}

void Poco::set_block_j(nlohmann::json block_j)
{
    block_j_ = block_j;
}

std::string Poco::get_hash_of_new_block()
{
    return hash_of_block_;
}

void Poco::set_hash_of_new_block(std::string block)
{
    Crypto crypto;
    hash_of_block_ = crypto.bech32_encode_sha256(block);
}

void Poco::reward_for_chosen_ones(std::string co_from_this_block, nlohmann::json chosen_ones_reward_j)
{
    // hello_reward req with nlohmann::json chosen_ones as argument
    // coordinator is hash of chosen_ones

    Crypto crypto;
    Rocksy* rocksy = new Rocksy("usersdb");
    std::string chosen_ones_s = chosen_ones_reward_j.dump();
    std::string hash_of_cos = crypto.bech32_encode_sha256(chosen_ones_s);
    std::string coordinator = rocksy->FindChosenOne(hash_of_cos);
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(coordinator));
    delete rocksy;
    
    uint32_t ip = contents_j["ip"];
    std::string ip_s;
    P2p p2p;
    p2p.number_to_ip_string(ip, ip_s);

    nlohmann::json message_j, to_sign_j;
    message_j["req"] = "hello_reward";
    message_j["full_hash_req"] = co_from_this_block;
    message_j["hash_of_block"] = get_hash_of_new_block();
    message_j["chosen_ones_reward"] = chosen_ones_reward_j;

    to_sign_j["req"] = message_j["req"];
    to_sign_j["full_hash_req"] = co_from_this_block;
    to_sign_j["hash_of_block"] = get_hash_of_new_block();
    to_sign_j["chosen_ones_reward"] = chosen_ones_reward_j;
    std::string to_sign_s = to_sign_j.dump();
    // std::cout << "to_sign_s: " << to_sign_s << std::endl;
    ECDSA<ECP, SHA256>::PrivateKey private_key;
    std::string signature;
    crypto.ecdsa_load_private_key_from_string(private_key);
    if (crypto.ecdsa_sign_message(private_key, to_sign_s, signature))
    {
        message_j["signature"] = crypto.base64_encode(signature);
    }

    std::string message_s = message_j.dump();

    std::cout << "Hello_reward request sent" << std::endl;

    P2pNetwork pn;
    pn.p2p_client(ip_s, message_s);
}

std::string Poco::hash_of_block_ = "";
nlohmann::json Poco::block_j_ = {};