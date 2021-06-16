#include "poco_c.hpp"

#include "merkle_tree.hpp"
#include "rocksy.hpp"
#include "prev_hash_c.hpp"
#include "protocol_c.hpp"
#include "merkle_tree_c.hpp"
#include "p2p_network.hpp"
#include "block_matrix.hpp"

#include <vector>
#include <algorithm>
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

    // The second part of the capstone implementation of poco:
    BlockMatrix bm;
    for (uint16_t i; i < bm.get_received_block_matrix().back().size(); i++)
    {
        /// base new_blocks on prev_blocks: prev_blocks --> decreasing txs --> count to 10
        // in the future there will be a lot of finetuning work on this function
        // preliminarly this is ok

        for (uint16_t j = tx_.get_transactions().size(); j > 0; j--) // Decrease the amount of transactions in the blocks
        {
            for (int k = 0; k < 10; k++) // Create 10 different blocks with the same number of included transactions
            {
                merkle_tree_c mt;

                std::vector<std::string> m_v;
                nlohmann::json entry_tx_j, entry_transactions_j;
                std::string full_hash_req;

                for (uint16_t l = 0; l < j; l++) // Add the transactions till the i-th transaction to the block
                {
                    m_v = *tx_.get_transactions().at(l).second;

                    full_hash_req = m_v[0]; // full_hash_req

                    entry_tx_j["full_hash_req"] = full_hash_req;
                    entry_tx_j["to_full_hash"] = m_v[1]; // to_full_hash
                    entry_tx_j["amount"] = m_v[2]; // amount
                    s_shptr_c_->push(entry_tx_j.dump());

                    entry_transactions_j.push_back(entry_tx_j);
                }

                s_shptr_c_ = mt.calculate_root_hash_c(s_shptr_c_);
                std::string datetime = mt.time_now_c();
                std::string root_hash_data = s_shptr_c_->top();
                block_j_c_ = mt.create_block_c(datetime, root_hash_data, entry_transactions_j);

                Crypto crypto;
                std::string the_block = bm.get_received_block_matrix().back()[i]->dump();
                block_j_c_["prev_hash"] = crypto.bech32_encode_sha256(the_block);

                ProtocolC proto;
                std::string my_last_block_nr = proto.get_last_block_nr_c();

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
                inform_chosen_ones_c(my_next_block_nr, block_j_c_, full_hash_req);

                // Add blocks to vector<vector<block_j_c_>>>
                bm.add_block_to_block_vector(block_j_c_);



                /// 2) intro_block and new_block???

                /// 3) create filtering_function(vvb)
                // read prev_hash'es from newest layer
                // evaluate prev_hash'es going to an older layer

                /// 4) a verification function of the blocks for when the application is started

                
            }
        }
    }

    bm.add_block_vector_to_block_matrix();
}

void PocoC::inform_chosen_ones_c(std::string my_next_block_nr, nlohmann::json block_j, std::string full_hash_req)
{
    FullHash fh;
    std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be

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
        std::cout << "Inform my fellow chosen_ones as coordinator coin" << std::endl;

        Protocol proto;
        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

        nlohmann::json to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "intro_block_c";
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

        // Update rocksdb --> take into account that rocksdb should be updated starting when there's 1 final block
        // for (int i = 0; i < tx_.get_transactions().size(); i++)
        // {
        //     nlohmann::json m_j;
        //     m_j = *message_j_vec_.get_message_j_vec()[i];

        //     std::string full_hash_req = m_j["full_hash_req"];
            
        //     Crypto crypto;
        //     // update rocksdb
            nlohmann::json rocksdb_j;
            rocksdb_j["funds"] = "jaja";
        //     std::string rocksdb_s = rocksdb_j.dump();

        //     // Store to rocksdb for coordinator
        //     Rocksy* rocksy = new Rocksy("usersdb");
        //     rocksy->Put(full_hash_req, rocksdb_s);
        //     delete rocksy;

        //     // Send rocksdb to into_block chosen_ones
            message_j["rocksdb"]/*[i]*/ = rocksdb_j;
        // }

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

            std::cout << "Preparation for intro_block coin: " << peer_ip << std::endl;

            std::string ip_from_peer;
            P2p p2p;
            p2p.number_to_ip_string(peer_ip, ip_from_peer);

            // p2p_client() to all chosen ones with intro_peer request
            pn.p2p_client(ip_from_peer, message);
        }

        merkle_tree_c mt;
        std::string block_s = mt.save_block_to_file_c(block_j, my_next_block_nr);

        ///set_hash_of_new_block(block_s); // TODO make a coin one of this function

        // Give the chosen_ones their reward:
        nlohmann::json chosen_ones_reward = message_j["chosen_ones"];
        reward_for_chosen_ones(co_from_this_block, chosen_ones_reward);
    }
    else
    {
        // You're not the coordinator!
        std::cout << "You're not the coordinator coin!" << std::endl;
    }
}

void PocoC::evaluate_transactions()
{
    // for every tx: lookup its payer's full_hash and compare with all the others, if duplicate then verify the resulting funds after every tx
    // double spending problem solution:
    Transactions tx;
    std::map<std::string, std::vector<std::string>> latest_transaction = tx.get_latest_transaction();

    std::string latest_tx_hash, latest_full_hash_req, latest_amount;
    std::vector<std::string> tx_hashes_vec, full_hashes_reqs_vec, amounts_vec;

    // Fill the vectors and the latest strings
    for (auto& lt: latest_transaction) {
        latest_tx_hash = lt.first;
        tx_hashes_vec.push_back(latest_tx_hash);
        latest_full_hash_req = lt.second.at(0);
        full_hashes_reqs_vec.push_back(latest_full_hash_req); // payer
        latest_amount = lt.second.at(2);
        amounts_vec.push_back(latest_amount);
    }

    std::map<std::string, std::map<std::string, std::string>> same_payer_payments; // <full_hash_req, <tx_hash, payment_in_this_tx>>
    for (uint16_t i = 0; i < full_hashes_reqs_vec.size(); i++)
    {
        if (full_hashes_reqs_vec[i] == latest_full_hash_req)
        {
            // more entries from same payer
            std::map<std::string, std::string> txamount;
            txamount[latest_tx_hash] = latest_amount;
            same_payer_payments[latest_full_hash_req] = txamount;
        }
    }

    std::map<std::string, std::string> total_same_payer_payments; // <full_hash_req, total_payment_in_this_block>
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

    // also for every tx: lookup funds in rocksy (blockchain must be verified!) and verify if they fulfill the tx
    Rocksy* rocksy = new Rocksy("transactionsdb");
    for (uint16_t i = 0; i < full_hashes_reqs_vec.size(); i++)
    {
        std::string full_hash_req = full_hashes_reqs_vec[i];
        nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(full_hash_req));
        uint64_t funds = contents_j["funds"];

        uint64_t amount;
        std::istringstream iss(amounts_vec[i]);
        iss >> amount;

        if (funds >= amount)
        {
            // Funds sufficient
            std::cout << "Funds sufficient" << std::endl;

            continue;
        }
        else
        {
            std::map<std::string, std::string>::iterator it;
            it = total_same_payer_payments.find(full_hash_req);

            uint64_t total_amount;
            std::istringstream iss(it->second);
            iss >> total_amount;

            if (it != total_same_payer_payments.end() && funds >= total_amount)
            {
                // Funds sufficient for multiple payments
                std::cout << "Funds sufficient for multiple payments" << std::endl;

                continue;
            }
            else
            {
                // Funds not sufficient
                std::cout << "Funds not sufficient" << std::endl; // TODO send message to payer and payee

                if (it != total_same_payer_payments.end())
                {
                    same_payer_payments.erase(full_hash_req);
                    total_same_payer_payments.erase(full_hash_req);
                }

                tx_hashes_vec.erase(tx_hashes_vec.begin() + i);
                full_hashes_reqs_vec.erase(full_hashes_reqs_vec.begin() + i);
                amounts_vec.erase(amounts_vec.begin() + i);
            }
        }
    }
    delete rocksy;
}

void PocoC::candidate_blocks_creation()
{
    //
}