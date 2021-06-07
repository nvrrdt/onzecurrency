#include "poco_c.hpp"

#include "merkle_tree.hpp"

#include <vector>
#include <sstream>

using namespace Coin;

void PocoC::create_and_send_block_c()
{
    // The capstone implemenation, an algorithm for block creation arithmetic:
    // 1) Evaluate Transactions (also double spend)
    //    - for every tx: lookup its payer's full_hash and compare with all the others, if duplicate then verify the resulting funds after every tx
    //    - also for every tx: lookup funds in rocksy (blockchain must be verified!) and verify if they correspond with the tx
    //      --> if those don't correspond, send message to payer and payee and prune that tx from Transactions
    // 2) Produce candidate blocks:
    //    - first ever iteration:
    //      --> there's only one block to create
    //      --> create 10 blocks, each with a different count from 0 to 9, each count creates another hash_of_new_block
    //      --> redo the previous step but then with 1 tx less included
    //      --> repeat this, reducing by 1 tx, until receiving a intro_block_c or new_block_c request, or until the block creation delay has passed
    //    - starting from the second iteration:
    //      --> numerous blocks as proposals for the final block
    //      --> create 10 blocks for every of the fastest 10 blocks of the previous iteration
    //      --> redo the previous step but then with 1 tx less included
    //      --> repeat this, reducing by 1 tx, until receiving a intro_block_c or new_block_c request, or until the block creation delay has passed
    //    - after a few iterations the fittest sole remainder will become the final block
    // 3) Send intro_block_c or new_block_c request
    //
    // Remarks:
    // * Be aware for the amount of memory these steps require!
    // * Somehow we're trying to prevent DDOS'ing in this step
    // * A headless state might be possible where incoming intro_block_c or new_block_c requests don't contain a prev_hash which you're able to reproduce:
    //   --> then the network should be questioned a few times until some satisfactory solution
    // * You should also pickup leftover transactions that weren't processed

    // The capstone implementation of poco:
    evaluate_transactions();
    candidate_blocks_creation();



//     merkle_tree mt;

//     nlohmann::json m_j, entry_tx_j, entry_transactions_j, exit_tx_j, exit_transactions_j;
//     nlohmann::json to_block_j;
//     std::string fh_s;
//     std::string full_hash_req;

//     for (int i = 0; i < tx_.get_transactions().size(); i++)
//     {
//         m_j = *tx_.get_transactions()[i];

//         full_hash_req = m_j["full_hash_req"];

//         to_block_j["full_hash"] = full_hash_req;
//         to_block_j["ecdsa_pub_key"] = m_j["ecdsa_pub_key"];
//         to_block_j["rsa_pub_key"] = m_j["rsa_pub_key"];
//         s_shptr_->push(to_block_j.dump());

//         entry_tx_j["full_hash"] = to_block_j["full_hash"];
//         entry_tx_j["ecdsa_pub_key"] = to_block_j["ecdsa_pub_key"];
//         entry_tx_j["rsa_pub_key"] = to_block_j["rsa_pub_key"];
//         entry_transactions_j.push_back(entry_tx_j);
//         exit_tx_j["full_hash"] = "";
//         exit_transactions_j.push_back(exit_tx_j);
//     }

//     s_shptr_ = mt.calculate_root_hash(s_shptr_);
//     std::string datetime = mt.time_now();
//     std::string root_hash_data = s_shptr_->top();
//     block_j_ = mt.create_block(datetime, root_hash_data, entry_transactions_j, exit_transactions_j);

//     PrevHash ph;
//     block_j_["prev_hash"] = ph.calculate_hash_from_last_block();

//     Protocol proto;
//     std::string my_last_block_nr = proto.get_last_block_nr();

//     std::string my_next_block_nr;
//     uint64_t value;
//     std::istringstream iss(my_last_block_nr);
//     iss >> value;
//     value++;
//     std::ostringstream oss;
//     oss << value;
//     my_next_block_nr = oss.str();

//     // send hash of this block with the block contents to the co's, forget save_block_to_file
//     // is the merkle tree sorted, then find the last blocks that are gathered for all the co's

//     // send intro_block to co's
//     inform_chosen_ones_c(my_next_block_nr, block_j_, full_hash_req);

//     message_j_vec_.reset_message_j_vec();
//     all_hashes_.reset_all_hashes();

// std::cout << "--------5c: " << std::endl;
}

void PocoC::inform_chosen_ones_c(std::string my_next_block_nr, nlohmann::json block_j, std::string full_hash_req)
{
    // FullHash fh;
    // std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be
    // // std::cout << "My_full_hash already present in file:__ " << my_full_hash << std::endl;

    // Crypto* crypto = new Crypto();
    // std::string block_s = block_j.dump();
    // std::string hash_of_block = crypto->bech32_encode_sha256(block_s);
    // delete crypto;
    // Rocksy* rocksy = new Rocksy("usersdb");
    // std::string co_from_this_block = rocksy->FindChosenOne(hash_of_block);
    // delete rocksy;

    // nlohmann::json message_j;

    // if (co_from_this_block == my_full_hash)
    // {
    //     // You are the coordinator!
    //     std::cout << "Inform my fellow chosen_ones as coordinator" << std::endl;

    //     Protocol proto;
    //     std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

    //     nlohmann::json to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
    //     message_j["req"] = "intro_block";
    //     message_j["latest_block_nr"] = my_next_block_nr;
    //     message_j["block"] = block_j;
    //     PrevHash ph;
    //     message_j["prev_hash"] = ph.calculate_hash_from_last_block();
    //     message_j["full_hash_req"] = full_hash_req;
    //     message_j["full_hash_coord"] = hash_of_block;

    //     int k;
    //     std::string v;
    //     for (auto &[k, v] : parts)
    //     {
    //         message_j["chosen_ones"].push_back(v);
    //     }

    //     // this for loop should be in inform_chosen_ones coordinator and send with an intro_block and a new_block
    //     // intro_block and new_block should put this in rocksdb
    //     // also a your_full_hash should receive and store this
    //     // and then the coordinator should put in rocksdb
    //     for (int i = 0; i < message_j_vec_.get_message_j_vec().size(); i++)
    //     {
    //         nlohmann::json m_j;
    //         m_j = *message_j_vec_.get_message_j_vec()[i];

    //         std::string full_hash_req = m_j["full_hash_req"];
            
    //         Crypto crypto;
    //         // update rocksdb
    //         nlohmann::json rocksdb_j;
    //         rocksdb_j["version"] = "O.1";
    //         rocksdb_j["ip"] = m_j["ip"];
    //         rocksdb_j["online"] = true;
    //         rocksdb_j["server"] = true;
    //         rocksdb_j["fullnode"] = true;
    //         std::string email_of_req = m_j["email_of_req"];
    //         rocksdb_j["hash_email"] = crypto.bech32_encode_sha256(email_of_req);
    //         rocksdb_j["prev_hash"] = ph.calculate_hash_from_last_block();
    //         rocksdb_j["full_hash"] = full_hash_req;
    //         Protocol proto;
    //         rocksdb_j["block_nr"] = proto.get_last_block_nr();
    //         rocksdb_j["ecdsa_pub_key"] = m_j["ecdsa_pub_key"];
    //         rocksdb_j["rsa_pub_key"] = m_j["rsa_pub_key"];
    //         std::string rocksdb_s = rocksdb_j.dump();

    //         // Store to rocksdb for coordinator
    //         Rocksy* rocksy = new Rocksy("usersdb");
    //         rocksy->Put(full_hash_req, rocksdb_s);
    //         delete rocksy;

    //         // Send rocksdb to into_block chosen_ones
    //         message_j["rocksdb"][i] = rocksdb_j;
    //     }

    //     to_sign_j["latest_block_nr"] = my_next_block_nr;
    //     to_sign_j["block"] = block_j;
    //     to_sign_j["prev_hash"] = ph.calculate_hash_from_last_block();
    //     to_sign_j["full_hash_req"] = full_hash_req;
    //     to_sign_j["full_hash_coord"] = hash_of_block;
    //     to_sign_j["chosen_ones"] = message_j["chosen_ones"];
    //     to_sign_j["rocksdb"] = message_j["rocksdb"];
    //     std::string to_sign_s = to_sign_j.dump();
    //     ECDSA<ECP, SHA256>::PrivateKey private_key;
    //     std::string signature;
    //     Crypto* crypto = new Crypto();
    //     crypto->ecdsa_load_private_key_from_string(private_key);
    //     if (crypto->ecdsa_sign_message(private_key, to_sign_s, signature))
    //     {
    //         message_j["signature"] = crypto->base64_encode(signature);
    //     }
    //     delete crypto;

    //     P2pNetwork pn;
    //     std::string key, val;
    //     for (auto &[key, val] : parts)
    //     {
    //         if (key == 1) continue;
    //         if (val == full_hash_req) continue;
    //         if (val == my_full_hash || val == "" || val == "0") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
            
    //         Rocksy* rocksy = new Rocksy("usersdb");

    //         // lookup in rocksdb
    //         nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
    //         uint32_t peer_ip = value_j["ip"];

    //         delete rocksy;
            
    //         std::string message = message_j.dump();

    //         std::cout << "Preparation for intro_block: " << peer_ip << std::endl;

    //         std::string ip_from_peer;
    //         P2p p2p;
    //         p2p.number_to_ip_string(peer_ip, ip_from_peer);

    //         // p2p_client() to all chosen ones with intro_peer request
    //         pn.p2p_client(ip_from_peer, message);
    //     }

    //     merkle_tree mt;
    //     std::string block_s = mt.save_block_to_file(block_j, my_next_block_nr);

    //     set_hash_of_new_block(block_s);

    //     // Send your_full_hash request to intro_peer's
    //     for (auto &[key, value] : all_hashes_.get_all_hashes())
    //     {
    //         nlohmann::json msg_j;
    //         msg_j["req"] = "your_full_hash";
    //         std::vector<std::string> vec = *value;
    //         msg_j["full_hash"] = vec[0];
    //         msg_j["prev_hash"] = vec[1];
    //         msg_j["block"] = block_j;
    //         Protocol proto;
    //         msg_j["block_nr"] = proto.get_last_block_nr();

    //         msg_j["rocksdb"] = message_j["rocksdb"];

    //         std::string msg_s = msg_j.dump();

    //         std::string peer_ip;
    //         P2p p2p;
    //         p2p.number_to_ip_string(key, peer_ip);
            
    //         std::cout << "_______key: " << key << " ip: " << peer_ip << ", value: " << value << std::endl;
    //         P2pNetwork pn;
    //         pn.p2p_client(peer_ip, msg_s);
    //     }

    //     // Give the chosen_ones their reward:
    //     nlohmann::json chosen_ones_reward = message_j["chosen_ones"];
    //     reward_for_chosen_ones(co_from_this_block, chosen_ones_reward);
    // }
    // else
    // {
    //     // You're not the coordinator!
    //     std::cout << "You're not the coordinator!" << std::endl;
    // }
}

void PocoC::evaluate_transactions()
{
    // for every tx: lookup its payer's full_hash and compare with all the others, if duplicate then verify the resulting funds after every tx
    // double spending problem solution:
    Transactions txs;
    std::map<std::string, std::shared_ptr<std::vector<std::string>>> transactions = txs.get_transactions();
    std::vector<std::string> tx_hashes_vec;
    std::vector<std::string> full_hashes_vec;
    std::vector<std::string> amounts_vec;

    for(std::map<std::string, std::shared_ptr<std::vector<std::string>>>::iterator iter = transactions.begin(); iter != transactions.end(); ++iter)
    {
        std::shared_ptr<std::vector<std::string>> content = iter->second;
        std::string tx_hash = iter->first;
        full_hashes_vec.push_back(tx_hash);
        std::string full_hash_req = content->at(0); // payer
        full_hashes_vec.push_back(full_hash_req);
        std::string amount = content->at(2);
        amounts_vec.push_back(amount);
    }

    std::map<std::string, std::map<std::string, std::string>> same_payer_payments;
    for (uint16_t i = 0; i < full_hashes_vec.size(); i++)
    {
        for (uint16_t j = 0; j < full_hashes_vec.size(); j++)
        {
            if (full_hashes_vec[i] == full_hashes_vec[j] && i != j)
            {
                // more entries from same payer
                std::map<std::string, std::string> txsamounts;
                txsamounts[tx_hashes_vec[i]] = amounts_vec[i];
                txsamounts[tx_hashes_vec[j]] = amounts_vec[j];
                same_payer_payments[full_hashes_vec[i]] = txsamounts;
            }
        }
    }

    std::map<std::string, std::string> total_same_payer_payments; // <full_hash, total_payment_in_this_block>
    for (std::map<std::string, std::map<std::string, std::string>>::iterator iter1 = same_payer_payments.begin(); iter1 != same_payer_payments.end(); ++iter1)
    {
        std::map<std::string, std::string> txsamounts = iter1->second;
        uint64_t payment = 0;
        for (std::map<std::string, std::string>::iterator iter2 = txsamounts.begin(); iter2 != txsamounts.end(); ++iter2)
        {
            // Add up all amounts for same payer for different transactions
            uint64_t amount;
            std::istringstream iss(iter2->second);
            iss >> amount;
            payment += amount;
        }

        std::ostringstream o;
        o << payment;
        total_same_payer_payments[iter1->first] = o.str();
    }

    // also for every tx: lookup funds in rocksy (blockchain must be verified!) and verify if they correspond with the tx
}

void PocoC::candidate_blocks_creation()
{
    //
}