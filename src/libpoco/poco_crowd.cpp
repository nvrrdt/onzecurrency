#include "poco_crowd.hpp"
#include "p2p.hpp"
#include "auth.hpp"
#include "crypto.hpp"
#include "p2p_network.hpp"
#include "merkle_tree.hpp"
#include "block_matrix.hpp"
#include "synchronisation.hpp"

using namespace Common;
using namespace Poco;

void PocoCrowd::create_and_send_block()
{
    // The capstone implemenation, an algorithm for block creation arithmetic:
    // 1) Evaluate NewPeers (in intro_msg_vec) (also take care of an analogy with double spend) (not implemented yet)
    //    - for every new_peer: lookup its hash_email and compare what is in rocksdb if it already exists
    // 2) Produce candidate blocks:
    //    - first ever iteration:
    //      --> there's only one block to create
    //      --> create 10 blocks, each with a different count from 0 to 9, each count creates another hash_of_new_block
    //      --> redo the previous step but then with 1 new_peer less included
    //      --> repeat this, reducing by 1 new_peer, until receiving a intro_block_c or new_block_c request, or until the block creation delay has passed
    //    - starting from the second iteration:
    //      --> numerous blocks as proposals for the final block
    //      --> create 10 blocks for every of the fastest 10 blocks of the previous iteration
    //      --> redo the previous step but then with 1 new_peer less included
    //      --> repeat this, reducing by 1 new_peer, until receiving a intro_block_c or new_block_c request, or until the block creation delay has passed
    //    - after a few iterations the fittest sole remainder will become the final block
    // 3) Send intro_block or new_block request
    //
    // Remarks:
    // * Be aware for the amount of memory these steps require!
    // * Somehow we're trying to prevent DDOS'ing in this step
    // * A headless state might be possible where incoming intro_block or new_block requests don't contain a prev_hash which you're able to reproduce:
    //   --> then the network should be questioned a few times until some satisfactory solution
    // * You should also pickup leftover new_peers that weren't processed

    // The second part of the capstone implementation of poco:
    std::cout << "create_and_send_block" << std::endl;

    BlockMatrix *bm = new BlockMatrix();
    Synchronisation sync;
    sync.set_break_block_creation_loops(false);

    nlohmann::json rocksdb_out;
    std::string my_next_block_nr;

    uint16_t limit_count = 0;

    if (bm->get_block_matrix().empty()) // TODO I think get_block_matrix is never empty
    {
        std::cout << "Crowd: No block_matrix: you're probably bootstrapping coin" << std::endl;           

        for (uint16_t j = intro_msg_vec_.get_intro_msg_vec().size(); j > 0; j--) // Decrease the amount of new_peers in the blocks
        {
            // TODO limit the reach of this loop otherwise the previous loop isn't usable

            std::cout << "Crowd: 2nd for loop " << j << std::endl;

            if (sync.get_break_block_creation_loops()) break;

            for (int nonce = 0; nonce < 10; nonce++) // Create 10 different blocks with the same number of included new_peers
            {

                std::cout << "Crowd: 3rd for loop " << nonce << std::endl;

                if (sync.get_break_block_creation_loops()) break;

                Crowd::merkle_tree *mt = new Crowd::merkle_tree();

                nlohmann::json imm, entry_tx_j, entry_transactions_j, exit_tx_j, exit_transactions_j;
                nlohmann::json to_block_j;
                std::string fh_s;
                std::string prel_full_hash_req;

                mt->set_genesis_prev_hash();
                std::string prel_prev_hash_req = mt->get_genesis_prev_hash();
                block_j_["prev_hash"] = prel_prev_hash_req;
                block_j_["nonce"] = nonce;

                for (int l = 0; l < j; l++) // Add the new_peers till the i-th new_peer to the block
                {
                    std::cout << "Crowd: 4th for loop " << l << std::endl;

                    imm = *intro_msg_vec_.get_intro_msg_vec().at(l);

                    Common::Crypto crypto;
                    std::string hash_of_email = imm["hash_of_email"];
                    std::string email_prev_hash_concatenated = hash_of_email + prel_prev_hash_req;
                    prel_full_hash_req =  crypto.bech32_encode_sha256(email_prev_hash_concatenated);

                    to_block_j["full_hash"] = prel_full_hash_req;
                    to_block_j["ecdsa_pub_key"] = imm["ecdsa_pub_key"];
                    to_block_j["rsa_pub_key"] = imm["rsa_pub_key"];
                    s_shptr_->push(to_block_j.dump());

                    entry_tx_j["full_hash"] = to_block_j["full_hash"];
                    entry_tx_j["ecdsa_pub_key"] = to_block_j["ecdsa_pub_key"];
                    entry_tx_j["rsa_pub_key"] = to_block_j["rsa_pub_key"];
                    entry_transactions_j.push_back(entry_tx_j);
                    exit_tx_j["full_hash"] = "";
                    exit_transactions_j.push_back(exit_tx_j);
                }

                s_shptr_ = mt->calculate_root_hash(s_shptr_);
                std::string datetime = mt->time_now();
                std::string root_hash_data = s_shptr_->top();
                block_j_ = mt->create_block(datetime, root_hash_data, entry_transactions_j, exit_transactions_j);

                Crowd::Protocol proto;
                std::string my_last_block_nr = proto.get_last_block_nr();

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
                inform_chosen_ones(my_next_block_nr, block_j_, prel_full_hash_req, rocksdb_out);

                // Add blocks to vector<vector<block_j_c_>>>
                bm->add_block_to_block_vector(block_j_);
                bm->add_calculated_hash_to_calculated_hash_vector(block_j_);
                bm->add_prev_hash_to_prev_hash_vector(block_j_);

                // Update rocksdb
                message_j["rocksdb"]["prev_hash"] = prel_prev_hash_req;
                message_j["rocksdb"]["full_hash"] = prel_full_hash_req;

                delete mt;

                limit_count++; // TODO this 100 is a variable that can be changed, there are others as well
                if (limit_count == 100) sync.set_break_block_creation_loops(true);
            }
            std::cout << "11 crowd" << std::endl;
        }
        std::cout << "22 crowd" << std::endl;
    }
    else
    {
        std::cout << "Crowd: Normal execution for block creation ..." << std::endl;           

        for (uint16_t i = 0; i < bm->get_block_matrix().back().size(); i++)
        {
            std::cout << "Crowd: 1st for loop with block matrix " << i << std::endl;

            if (sync.get_break_block_creation_loops()) break;

            /// base new_blocks on prev_blocks: prev_blocks --> decreasing txs --> count to 10
            // in the future there will be a lot of finetuning work on this function
            // preliminarly this is ok

            for (uint16_t j = intro_msg_vec_.get_intro_msg_vec().size(); j > 0; j--) // Decrease the amount of transactions in the blocks
            {
                std::cout << "Crowd: 2nd for loop with block matrix " << j << std::endl;

                if (sync.get_break_block_creation_loops()) break;

                // TODO limit the reach of this loop otherwise the previous loop isn't usable

                for (int nonce = 0; nonce < 10; nonce++) // Create 10 different blocks with the same number of included transactions
                {
                    std::cout << "Crowd: 3rd for loop with block matrix " << nonce << std::endl;

                    if (sync.get_break_block_creation_loops()) break;

                    Crowd::merkle_tree *mt = new Crowd::merkle_tree();

                    nlohmann::json imm, entry_tx_j, entry_transactions_j, exit_tx_j, exit_transactions_j;
                    nlohmann::json to_block_j;
                    std::string fh_s;
                    std::string prel_full_hash_req;
                    std::string prel_prev_hash_req = *bm->get_calculated_hash_matrix().back().at(i);

                    for (int l = 0; l < j; l++) // Add the transactions till the i-th transaction to the block
                    {
                        std::cout << "Crowd: 4th for loop with block matrix " << l << std::endl;
                        
                        imm = *intro_msg_vec_.get_intro_msg_vec().at(l);

                        Common::Crypto crypto;
                        std::string hash_of_email = imm["hash_of_email"];
                        std::string email_prev_hash_concatenated = hash_of_email + prel_prev_hash_req;
                        prel_full_hash_req =  crypto.bech32_encode_sha256(email_prev_hash_concatenated);

                        to_block_j["full_hash"] = prel_full_hash_req;
                        to_block_j["ecdsa_pub_key"] = imm["ecdsa_pub_key"];
                        to_block_j["rsa_pub_key"] = imm["rsa_pub_key"];
                        s_shptr_->push(to_block_j.dump());

                        entry_tx_j["full_hash"] = to_block_j["full_hash"];
                        entry_tx_j["ecdsa_pub_key"] = to_block_j["ecdsa_pub_key"];
                        entry_tx_j["rsa_pub_key"] = to_block_j["rsa_pub_key"];
                        entry_transactions_j.push_back(entry_tx_j);
                        exit_tx_j["full_hash"] = "";
                        exit_transactions_j.push_back(exit_tx_j);
                    }

                    s_shptr_ = mt->calculate_root_hash(s_shptr_);
                    std::string datetime = mt->time_now();
                    std::string root_hash_data = s_shptr_->top();
                    block_j_ = mt->create_block(datetime, root_hash_data, entry_transactions_j, exit_transactions_j);


                    block_j_["prev_hash"] = prel_prev_hash_req;
                    block_j_["nonce"] = nonce;

                    Crowd::Protocol proto;
                    std::string my_last_block_nr = proto.get_last_block_nr();

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
                    inform_chosen_ones(my_next_block_nr, block_j_, prel_full_hash_req, rocksdb_out);

                    // Add blocks to vector<vector<block_j_c_>>>
                    bm->add_block_to_block_vector(block_j_);
                    bm->add_calculated_hash_to_calculated_hash_vector(block_j_);
                    bm->add_prev_hash_to_prev_hash_vector(block_j_);

                    // Update rocksdb
                    message_j["rocksdb"]["prev_hash"] = prel_prev_hash_req;
                    message_j["rocksdb"]["full_hash"] = prel_full_hash_req;

                    delete mt;

                    limit_count++; // TODO this 100 is a variable that can be changed, there are others as well
                    if (limit_count == 100) sync.set_break_block_creation_loops(true);
                }
            }
        }
    }

    // TODO your_full_hash must only be sent when a block is final!!

    // Send your_full_hash request to intro_peer's
    // for (auto &[key, value] : all_hashes_vec_.get_all_hashes_vec())
    // {
    //     nlohmann::json msg_j;
    //     msg_j["req"] = "your_full_hash";
    //     std::vector<std::string> vec = *value;
    //     msg_j["full_hash"] = vec[0];
    //     msg_j["prev_hash"] = vec[1];
    //     msg_j["block"] = block_j_;
    //     msg_j["block_nr"] = my_next_block_nr;

    //     msg_j["rocksdb"] = rocksdb_out;

    //     std::string msg_s = msg_j.dump();

    //     std::string peer_ip;
    //     Crowd::P2p p2p;
    //     p2p.number_to_ip_string(key, peer_ip);
        
    //     std::cout << "_______key: " << key << " ip: " << peer_ip << ", value: " << value << std::endl;
    //     Crowd::P2pNetwork pn;
    //     pn.p2p_client(peer_ip, msg_s);
    // }

    intro_msg_vec_.reset_intro_msg_vec(); // TODO put these two in teh beginning of this function
    ip_hemail_vec_.reset_ip_hemail_vec();       // and make a copy of intro_msg_vec_ and all_hashes_vec_, work with thes copies here
                                          // --> new intro_peers might arrive and shouldn't be taken into account

    // fill the matrices
    bm->add_block_vector_to_block_matrix();
    bm->add_calculated_hash_vector_to_calculated_hash_matrix();
    bm->add_prev_hash_vector_to_prev_hash_matrix();

    bm->sifting_function_for_both_block_matrices();
std::cout << "1111" << std::endl;
    bm->save_final_block_to_file();
std::cout << "2222" << std::endl;

    // for debugging purposes:
    for (int i = 0; i < bm->get_block_matrix().size(); i++)
    {
        for (int j = 0; j < bm->get_block_matrix().at(i).size(); j++)
        {
            nlohmann::json content_j = *bm->get_block_matrix().at(i).at(j);
            std::cout << "block matrix entry " << i << " " << j << " (oldest first)" << std::endl;
        }
    }

    delete bm;

std::cout << "--------5: " << std::endl;
}

void PocoCrowd::inform_chosen_ones(std::string my_next_block_nr, nlohmann::json block_j, std::string full_hash_req, nlohmann::json rocksdb_out)
{
    Crowd::FullHash fh;
    std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be
    // std::cout << "My_full_hash already present in file:__ " << my_full_hash << std::endl;

    Crypto* crypto = new Crypto();
    std::string block_s = block_j.dump();
    std::string hash_of_block = crypto->bech32_encode_sha256(block_s);
    delete crypto;
    Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdb");
    std::string co_from_this_block = rocksy->FindChosenOne(hash_of_block);
    delete rocksy;

    nlohmann::json message_j;

    if (co_from_this_block == my_full_hash)
    {
        // You are the coordinator!
        std::cout << "Inform my fellow chosen_ones as coordinator" << std::endl;

        Crowd::Protocol proto;
        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

        nlohmann::json to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "intro_prel_block";
        message_j["latest_block_nr"] = my_next_block_nr;
        message_j["block"] = block_j;
        Crowd::PrevHash ph;
        message_j["prev_hash"] = ph.calculate_hash_from_last_block();
        message_j["full_hash_req"] = full_hash_req;
        message_j["full_hash_coord"] = hash_of_block;

        int k;
        std::string v;
        for (auto &[k, v] : parts)
        {
            message_j["chosen_ones"].push_back(v);
        }

        // // this for loop should be in inform_chosen_ones coordinator and send with an intro_block and a new_block
        // // intro_block and new_block should put this in rocksdb
        // // also a your_full_hash should receive and store this
        // // and then the coordinator should put in rocksdb
        // for (int i = 0; i < intro_msg_vec_.get_intro_msg_vec().size(); i++)
        // {
        //     nlohmann::json m_j;
        //     m_j = *intro_msg_vec_.get_intro_msg_vec()[i];

        //     std::string full_hash_req = m_j["full_hash_req"];
            
        //     Common::Crypto crypto;
        //     // update rocksdb
        //     nlohmann::json rocksdb_j;
        //     rocksdb_j["version"] = "O.1";
        //     rocksdb_j["ip"] = m_j["ip"];
        //     rocksdb_j["online"] = true;
        //     rocksdb_j["server"] = true;
        //     rocksdb_j["fullnode"] = true;
        //     std::string email_of_req = m_j["email_of_req"];
        //     rocksdb_j["hash_email"] = crypto.bech32_encode_sha256(email_of_req);
        //     rocksdb_j["prev_hash"] = ph.calculate_hash_from_last_block();
        //     rocksdb_j["full_hash"] = full_hash_req;
        //     Crowd::Protocol proto;
        //     rocksdb_j["block_nr"] = proto.get_last_block_nr();
        //     rocksdb_j["ecdsa_pub_key"] = m_j["ecdsa_pub_key"];
        //     rocksdb_j["rsa_pub_key"] = m_j["rsa_pub_key"];
        //     std::string rocksdb_s = rocksdb_j.dump();

        //     // Store to rocksdb for coordinator
        //     // Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdb");
        //     // rocksy->Put(full_hash_req, rocksdb_s);
        //     // delete rocksy;

        //     // Send rocksdb to into_block chosen_ones
        //     message_j["rocksdb"][i] = rocksdb_j;
        // }

        to_sign_j["latest_block_nr"] = my_next_block_nr;
        to_sign_j["block"] = block_j;
        to_sign_j["prev_hash"] = ph.calculate_hash_from_last_block();
        to_sign_j["full_hash_req"] = full_hash_req;
        to_sign_j["full_hash_coord"] = hash_of_block;
        to_sign_j["chosen_ones"] = message_j["chosen_ones"];
        // to_sign_j["rocksdb"] = message_j["rocksdb"];
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

        Crowd::P2pNetwork pn;
        std::string key, val;
        for (auto &[key, val] : parts)
        {
            if (key == 1) continue;
            if (val == full_hash_req) continue;
            if (val == my_full_hash || val == "" || val == "0") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
            
            Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdb");

            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
            uint32_t peer_ip = value_j["ip"];

            delete rocksy;
            
            std::string message = message_j.dump();

            std::cout << "Preparation for intro_block: " << peer_ip << std::endl;

            std::string ip_from_peer;
            Crowd::P2p p2p;
            p2p.number_to_ip_string(peer_ip, ip_from_peer);

            // p2p_client() to all chosen ones with intro_peer request
            pn.p2p_client(ip_from_peer, message);
        }

        // rocksdb_out = message_j["rocksdb"];

        // Give the chosen_ones their reward:
        // nlohmann::json chosen_ones_reward = message_j["chosen_ones"];        // preliminary commented out
        // reward_for_chosen_ones(co_from_this_block, chosen_ones_reward);      // this line too
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

nlohmann::json PocoCrowd::get_block_j()
{
    // TODO intro_peer doesn't know its full_hash until new_block is send
    // but the other users don't know the whereabouts of intro_peer

    return block_j_;
}

void PocoCrowd::set_block_j(nlohmann::json block_j)
{
    block_j_ = block_j;
}

std::string PocoCrowd::get_hash_of_new_block()
{
    return hash_of_block_;
}

void PocoCrowd::set_hash_of_new_block(std::string block)
{
    Common::Crypto crypto;
    hash_of_block_ = crypto.bech32_encode_sha256(block);
}

void PocoCrowd::reward_for_chosen_ones(std::string co_from_this_block, nlohmann::json chosen_ones_reward_j)
{
    // hello_reward req with nlohmann::json chosen_ones as argument
    // coordinator is hash of chosen_ones

    Common::Crypto crypto;
    Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdb");
    std::string chosen_ones_s = chosen_ones_reward_j.dump();
    std::string hash_of_cos = crypto.bech32_encode_sha256(chosen_ones_s);
    std::string coordinator = rocksy->FindChosenOne(hash_of_cos);
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(coordinator));
    delete rocksy;
    
    uint32_t ip = contents_j["ip"];
    std::string ip_s;
    Crowd::P2p p2p;
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

    Crowd::P2pNetwork pn;
    pn.p2p_client(ip_s, message_s);
}

std::string PocoCrowd::hash_of_block_ = "";
nlohmann::json PocoCrowd::block_j_ = {};