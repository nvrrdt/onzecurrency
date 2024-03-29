#include "poco_crowd.hpp"
#include "p2p.hpp"
#include "auth.hpp"
#include "crypto.hpp"
#include "p2p_network.hpp"
#include "merkle_tree.hpp"
#include "block_matrix.hpp"
#include "synchronisation.hpp"
#include "sharding.hpp"

#include "print_or_log.hpp"

using namespace Common;
using namespace Poco;

std::vector<std::string> Poco::PocoCrowd::new_users_ip_ = {};

void PocoCrowd::create_prel_blocks()
{
    // The capstone implemenation, an algorithm for block creation arithmetic:
    // 1) Evaluate NewPeers (in intro_msg_map) (also take care of an analogy with double spend) (not implemented yet)
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
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Create prel_blocks"});

pl.handle_print_or_log({"____0000 cpb"});
    Synchronisation sync;

    nlohmann::json rocksdb_out;
    std::string my_next_block_nr;

    std::string datetime = sync.get_datetime_now();
pl.handle_print_or_log({"____0002 cpb"});
    BlockMatrix *bm = new BlockMatrix();
    bm->clear_new_users();
    clear_new_users_ip();
pl.handle_print_or_log({"____0002 cpb"});
    // create copies of these vectors and reset the original
    auto copy_ip_hemail_vec(ip_hemail_vec_.get_all_ip_hemail_vec());
    auto copy_intro_msg_map(intro_msg_map_.get_intro_msg_map());
    ip_hemail_vec_.reset_ip_hemail_vec();
    intro_msg_map_.reset_intro_msg_map();
pl.handle_print_or_log({"____0003 cpb"});

    /*
     * start the sifting process and save a final block
     * --> must be in beginning of loops at start of block_creation_delay
     * --> because of synchronisation or so, I think somehow new blocks need to be communicated at the start of block_creation and not afterwards
     */

    if (copy_intro_msg_map.size() == 0)
    {
        // Add an empty block
        block_j_ = { "entry", "false" };
        bm->add_block_to_block_vector(block_j_);
        bm->add_block_vector_to_block_matrix();
        copy_ip_hemail_vec.clear();
        copy_intro_msg_map.clear();

        return;
    } // TODO: make an else statement
    if (bm->sifting_function_for_both_block_matrices()) bm->save_final_block_to_file(); // --> does this save correctly?

    /**
     * What needs to be done in poco v2:
     * - create a block from the txs in a shard
     * - create also a block while removing txs from the vector in the shard
     * 
     */

    Crowd::merkle_tree *mt = new Crowd::merkle_tree();

    std::vector<std::shared_ptr<nlohmann::json>> txs_bucket_vec;
    nlohmann::json imv_j, entry_tx_j, entry_transactions_j, exit_tx_j, exit_transactions_j;
    nlohmann::json to_block_j;
    std::string fh_s;
    std::string prel_full_hash_req;
pl.handle_print_or_log({"____0000 2th"});
    // std::string prel_prev_hash_req = *bm->get_calculated_hash_matrix().back().at(i);
pl.handle_print_or_log({"____0001 2th"});
    Crowd::P2p p2p;
    std::string ip_quad;

    /**
     * - a block is created per shard --> are preliminary blocks still necessary?
     * - the FindShardChosenOnes() and then communicate the blocks to all users the poco v1 way
     * - finalizing the block happens later at the next block, then the prev_hash is known
     * - adapt the above comments
     * 
     * - a finalize function must be added where the prev_hash is added
     * - inform_chosen_ones_prel_block() (must be renamed) sends a block without prev_hash
     *   while the prev_hashes from a block_creation_delay earlier must be communicated and validated
     * - the sifting function must run independently every start of block_creation_delay,
     *   sifting only for parallel blocks with different amount of transactions
     * 
     */

    // Which shard?
    Poco::DatabaseSharding ds;
    auto shard_number = ds.which_shard_to_process();
pl.handle_print_or_log({"____0001.1 2th", std::to_string(shard_number), std::to_string(copy_intro_msg_map.size())});
    // Get transactions from correct shard
    auto it = copy_intro_msg_map.begin();
    std::advance(it, shard_number);
    txs_bucket_vec = it->second;
pl.handle_print_or_log({"____0001.2 2th"});
    // Start of the loops
    for (uint32_t i = txs_bucket_vec.size(); i > 0; i--) // Amount of comprised transactions differs
    {
        pl.handle_print_or_log({"Crowd: 1th for loop", std::to_string(i)});

        for (int j = 0; j < i; j++) // Add the transactions till the j-th transaction to the block
        {
            pl.handle_print_or_log({"Crowd: 2nd for loop", std::to_string(j)});

            imv_j = *txs_bucket_vec.at(j);
                
            // link an ip to a user
            std::shared_ptr<std::pair<std::string, std::string>> ip_hemail = copy_ip_hemail_vec.at(j);
            ip_all_hashes_.add_ip_hemail_to_ip_all_hashes_vec(ip_hemail);

pl.handle_print_or_log({"____0000 3th"});
//                 // update rocksdb
//                 imv_j["rocksdb"]["prev_hash"] = prel_prev_hash_req;
//                 imv_j["rocksdb"]["full_hash"] = prel_full_hash_req;
            intro_msg_s_mat_.add_intro_msg_to_intro_msg_s_vec(imv_j);
pl.handle_print_or_log({"____0001 3th"});
            // create block
            to_block_j["full_hash"] = "incomplete";
            to_block_j["ecdsa_pub_key"] = imv_j["ecdsa_pub_key"];
            to_block_j["rsa_pub_key"] = imv_j["rsa_pub_key"];
            s_shptr_->push(to_block_j.dump());
pl.handle_print_or_log({"____0001.1 3th"});
            entry_tx_j["full_hash"] = to_block_j["full_hash"];
            entry_tx_j["ecdsa_pub_key"] = to_block_j["ecdsa_pub_key"];
            entry_tx_j["rsa_pub_key"] = to_block_j["rsa_pub_key"];
            entry_transactions_j.push_back(entry_tx_j);
            exit_tx_j["full_hash"] = "";
            exit_transactions_j.push_back(exit_tx_j);
pl.handle_print_or_log({"____0001.2 3th"});
            // Sometimes new_prel_block() is received before your_full_hash is received
            // and then it doesn't work
            add_to_new_users_ip((*ip_hemail).first);
            // bm->add_to_new_users(prel_full_hash_req);
pl.handle_print_or_log({"____0001.3 3th"});
        }

        s_shptr_ = mt->calculate_root_hash(s_shptr_);
pl.handle_print_or_log({"____0001.4 3th"});
        std::string root_hash_data = s_shptr_->top();
pl.handle_print_or_log({"____0001.5 3th"});
pl.handle_print_or_log({"____0001.5 3th", datetime});
pl.handle_print_or_log({"____0001.5 3th", root_hash_data});
pl.handle_print_or_log({"____0001.5 3th", entry_transactions_j.dump()}); // full_hash is missing
        block_j_ = mt->create_block(datetime, root_hash_data, entry_transactions_j, exit_transactions_j);

pl.handle_print_or_log({"____0002 3th"});
        // block_j_["prev_hash"] = prel_prev_hash_req;
pl.handle_print_or_log({"____0003 3th"});
        Crowd::Protocol proto;
        std::string my_last_block_nr = proto.get_last_block_nr();
pl.handle_print_or_log({"____0003.1 3th"});
        uint64_t value;
        std::istringstream iss(my_last_block_nr);
        iss >> value;
        value++;
        std::ostringstream oss;
        oss << value;
        my_next_block_nr = oss.str();
pl.handle_print_or_log({"____0003.2 3th"});
        // send hash of this block with the block contents to the co's, forget save_block_to_file
        // is the merkle tree sorted, then find the last blocks that are gathered for all the co's

        // send intro_block to co's
        inform_network_prel_block(my_next_block_nr, block_j_); // sending prev_hashes for finalization and sending prel_blocks
pl.handle_print_or_log({"____0004 3th"});
        // Add blocks to vector<vector<block_j_>>
        bm->add_block_to_block_vector(block_j_);
        // bm->add_calculated_hash_to_calculated_hash_vector(block_j_);
        // bm->add_prev_hash_to_prev_hash_vector(block_j_);
pl.handle_print_or_log({"____0005 3th"});
        // Update rocksdb and prepare your_full_hash
        intro_msg_s_mat_.add_intro_msg_s_vec_to_intro_msg_s_2d_mat();
        ip_all_hashes_.add_ip_all_hashes_vec_to_ip_all_hashes_2d_mat();
pl.handle_print_or_log({"____0006 3th"});
        delete mt;
    }

    // clear these vectors
    copy_ip_hemail_vec.clear();
    copy_intro_msg_map.clear();
pl.handle_print_or_log({"____0007 3th"});
    // fill the matrices
    bm->add_block_vector_to_block_matrix();
    // bm->add_calculated_hash_vector_to_calculated_hash_matrix();
    // bm->add_prev_hash_vector_to_prev_hash_matrix();
pl.handle_print_or_log({"____0008 3th"});
    intro_msg_s_mat_.add_intro_msg_s_2d_mat_to_intro_msg_s_3d_mat();
    ip_all_hashes_.add_ip_all_hashes_2d_mat_to_ip_all_hashes_3d_mat();
pl.handle_print_or_log({"____0009 3th"});
    // for debugging purposes:
    for (int i = 0; i < bm->get_block_matrix().size(); i++)
    {
        for (int j = 0; j < bm->get_block_matrix().at(i).size(); j++)
        {
            nlohmann::json content_j = *bm->get_block_matrix().at(i).at(j);
            pl.handle_print_or_log({"block matrix entry", std::to_string(i), std::to_string(j), "(oldest first)"});
        }
    }

    delete bm;

pl.handle_print_or_log({"--------5:"});
}

void PocoCrowd::inform_network_prel_block(std::string my_next_block_nr, nlohmann::json block_j)
{
    /**
     * Send prev_hashes and prel_block to everyone through the chosen_ones
     * Amount of shards has nothing to do with amount of chosen_ones
     * 
     */

    Crowd::FullHash fh;
    std::string my_full_hash = fh.get_full_hash();
    Common::Print_or_log pl;
    // pl.handle_print_or_log({"My_full_hash already present in file:__ ", my_full_hash});
pl.handle_print_or_log({"____0000 inf"});
    Crypto* crypto = new Crypto();
    std::string block_s = block_j.dump();
    std::string bech32 = crypto->bech32_encode_sha256(block_s);
pl.handle_print_or_log({"____0000.1 inf", bech32});
    delete crypto;
    Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");
    std::vector<std::string> shard_chosen_ones_for_this_block = rocksy->FindShardChosenOnes(bech32);
    delete rocksy;
pl.handle_print_or_log({"____0001 inf"});
    nlohmann::json message_j;
    Synchronisation sync;

    bool present = false;
    for (auto &co: shard_chosen_ones_for_this_block)
    {
        if (co == my_full_hash) present = true;
    }
pl.handle_print_or_log({"____0002 inf"});
    if (present)
    {
        // You are the coordinator!
        pl.handle_print_or_log({"Inform my fellow chosen_ones as coordinator"});

        Crowd::Protocol proto;
        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);
pl.handle_print_or_log({"____0003 inf"});
        nlohmann::json to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "intro_prel_block";
        message_j["latest_block_nr"] = my_next_block_nr;
        message_j["block"] = block_j;
        Poco::BlockMatrix bm;
        message_j["prev_hash"] = *bm.get_calculated_hash_matrix().front().at(0); // --> could also be (front() = at(1)) --> it's a guess now
        message_j["full_hash_coord"] = my_full_hash;
pl.handle_print_or_log({"____0004 inf"});
        int k;
        std::string v;
        for (auto &[k, v] : parts)
        {
            if (v == "0" || v == "") break; // TODO the parts need to be refactored everywhere as it's an ugly hack

            message_j["chosen_ones"].push_back(v);
        }
pl.handle_print_or_log({"____0005 inf", message_j["chosen_ones"].dump()});
        to_sign_j["latest_block_nr"] = my_next_block_nr;
        to_sign_j["block"] = block_j;
        to_sign_j["prev_hash"] = message_j["prev_hash"];
        to_sign_j["full_hash_coord"] = my_full_hash;
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

        Network::P2pNetwork pn;
        std::string key, val;
        // bool is_part = false;
        for (auto &[key, val] : parts)
        {
//             for (auto& element: bm.get_new_users()) // --> new user's full_hash isn't know at this time
//             {
// pl.handle_print_or_log({"____0005.1 inf", element});
//                 if (val == element)
//                 {
//                     is_part = true;
//                 }
//             }
// pl.handle_print_or_log({"____0005.2 inf"});
//             if (is_part)
//             {
//                 is_part = false;
//                 continue;
//             }
pl.handle_print_or_log({"____0005.3 inf"});
            Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");
pl.handle_print_or_log({"____0006 inf"});
            // lookup in rocksdb
pl.handle_print_or_log({"____0006.1 inf", val});
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
pl.handle_print_or_log({"____0006.2 inf", value_j.dump()});
            std::string peer_ip = value_j["ip"];
pl.handle_print_or_log({"____0007 inf"});
            delete rocksy;
            
            std::string message = message_j.dump();

            pl.handle_print_or_log({"Preparation for intro_prel_block:", peer_ip});

            // p2p_client() to all chosen ones with intro_peer request
            pn.p2p_client(peer_ip, message);
        }

        // Should also fill the sent block vector
        bm.add_sent_block_to_sent_block_vector(block_j_);
    }
    else
    {
        // You're not the coordinator!
        pl.handle_print_or_log({"You're not the coordinator!"});
    }
}


void PocoCrowd::inform_network_final_block(nlohmann::json final_block_j, std::string new_block_nr, nlohmann::json rocksdb_j)
{
    Common::Print_or_log pl;

    Crowd::FullHash fh;
    std::string my_full_hash = fh.get_full_hash(); // TODO this is a file lookup and thus takes time --> static var should be

    Crypto* crypto = new Crypto();
    std::string block_s = final_block_j.dump();
    std::string bech32 = crypto->bech32_encode_sha256(block_s);
    delete crypto;
    Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");

    std::vector<std::string> shard_chosen_ones_for_this_block = rocksy->FindShardChosenOnes(bech32);
    delete rocksy;

    nlohmann::json message_j;

    bool present = false;
    for (auto &co: shard_chosen_ones_for_this_block)
    {
        if (co == my_full_hash) present = true;
    }

    if (present)
    {
        // You are the coordinator!
        pl.handle_print_or_log({"Inform my fellow chosen_ones as final coordinator"});

        Crowd::Protocol proto;
        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

        nlohmann::json to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "intro_final_block";
        message_j["latest_block_nr"] = new_block_nr;
        message_j["block"] = final_block_j;
        message_j["full_hash_coord"] = my_full_hash;

        message_j["rocksdb"] = rocksdb_j;

        int k;
        std::string v;
        for (auto &[k, v] : parts)
        {
            message_j["chosen_ones"].push_back(v);
        }

        to_sign_j["latest_block_nr"] = new_block_nr;
        to_sign_j["block"] = final_block_j;
        to_sign_j["full_hash_coord"] = message_j["full_hash_coord"];
        to_sign_j["rocksdb"] = message_j["rocksdb"];
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

        // send req to chosen_ones
        Network::P2pNetwork pn;
        std::string key, val;
        Poco::BlockMatrix bm;
        bool is_part;
        for (auto &[key, val] : parts)
        {
            for (auto& element: bm.get_new_users()) // --> new user's full_hash might be known at this point --> find out
            {
                if (val == element)
                {
                    is_part = true;
                }
            }
            if (is_part)
            {
                is_part = false;
                continue;
            }

            Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");

            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
            std::string peer_ip = value_j["ip"];

            delete rocksy;
            
            std::string message = message_j.dump();

            // p2p_client() to all chosen ones with intro_peer request
            pn.p2p_client(peer_ip, message);
        }

        // Give the chosen_ones their reward:
        // nlohmann::json chosen_ones_reward = message_j["chosen_ones"];        // preliminary commented out
        // reward_for_chosen_ones(co_from_this_block, chosen_ones_reward);      // this line too
    }
    else
    {
        // You're not the final coordinator!
        pl.handle_print_or_log({"You're not the final coordinator!"});
    }
}

void PocoCrowd::send_your_full_hash(uint16_t place_in_mat, nlohmann::json final_block_j, std::string new_block_nr)
{
    // your_full_hash must only be sent when a block is final!!

    Common::Print_or_log pl;
// pl.handle_print_or_log({"intro_msg_s", std::to_string(intro_msg_s_mat_.get_intro_msg_s_3d_mat().size())});
// pl.handle_print_or_log({"intro_msg_s", std::to_string(intro_msg_s_mat_.get_intro_msg_s_3d_mat().at(place_in_mat).size())});
// pl.handle_print_or_log({"intro_msg_s", std::to_string(intro_msg_s_mat_.get_intro_msg_s_3d_mat().at(place_in_mat).at(0).size())});

    Crowd::FullHash fh;
    std::string my_full_hash = fh.get_full_hash(); // TODO this is a file lookup and thus takes time --> static var should be

    Crypto* crypto = new Crypto();
    std::string block_s = final_block_j.dump();
    std::string bech32 = crypto->bech32_encode_sha256(block_s);
    delete crypto;
    Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");
    std::string co_from_this_block = rocksy->FindCoordinator(bech32);
    bool is_part = false;
    BlockMatrix bm;
    for (;;)
    {
        // it breaks when the new users (from a new block) are yet part of all the users
        for (auto& element: bm.get_new_users()) // --> dunno if user's full_hash is known at this point
        {
            if (co_from_this_block == element)
            {
                is_part = true;
            }
        }

        if (is_part)
        {
            co_from_this_block = rocksy->FindNextPeer(co_from_this_block);
            is_part = false;
        }
        else
        {
            break;
        }
    }
    delete rocksy;

    nlohmann::json message_j;

    if (co_from_this_block == my_full_hash)
    {
        // Send your_full_hash request to intro_peer's
        for (uint16_t i = 0; i < intro_msg_s_mat_.get_intro_msg_s_3d_mat().at(place_in_mat).at(0).size(); i++)
        {
            nlohmann::json msg_j;
            msg_j["req"] = "your_full_hash";
            nlohmann::json val_j = *intro_msg_s_mat_.get_intro_msg_s_3d_mat().at(place_in_mat).at(0).at(i);
            msg_j["full_hash"] = val_j["rocksdb"]["full_hash"];
            msg_j["prev_hash"] = val_j["rocksdb"]["prev_hash"];
            msg_j["block"] = final_block_j;
            msg_j["block_nr"] = new_block_nr;

            msg_j["rocksdb"] = val_j["rocksdb"];

            std::string msg_s = msg_j.dump();

            std::pair<std::string, std::string> ip_quad = *ip_all_hashes_.get_ip_all_hashes_3d_mat().at(place_in_mat).at(0).at(i);
            
            pl.handle_print_or_log({"_______key:", std::to_string(i), ", ip:", ip_quad.first});
            Network::P2pNetwork pn;
            pn.p2p_client(ip_quad.first, msg_s);
        }

        // // for debugging purposes:
        // Poco::IpAllHashes iah;
        // for (int i = 0; i < iah.get_ip_all_hashes_3d_mat().size(); i++)
        // {
        //     for (int j = 0; j < iah.get_ip_all_hashes_3d_mat().at(i).size(); j++)
        //     {
        //         for (int k = 0; k < iah.get_ip_all_hashes_3d_mat().at(i).at(j).size(); k++)
        //         {
        //             auto content = *iah.get_ip_all_hashes_3d_mat().at(i).at(j).at(k);
        //             pl.handle_print_or_log({"all hashes entry" std::to_string(i), std::to_string(j) std::to_string(k), std::to_string(content.first), "(oldest first)"});
        //         }
        //     }
        // }

        pl.handle_print_or_log({"Your_full_hash sent"});
    }
    else
    {
        pl.handle_print_or_log({"Your_full_hash not sent!", "mfh", my_full_hash, "coftb", co_from_this_block});
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

    Common::Print_or_log pl;

    Common::Crypto crypto;
    Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");
    std::string chosen_ones_s = chosen_ones_reward_j.dump();
    std::string bech32 = crypto.bech32_encode_sha256(chosen_ones_s);
    std::string coordinator = rocksy->FindCoordinator(bech32);
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(coordinator));
    delete rocksy;
    
    std::string ip = contents_j["ip"];
    
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
    // pl.handle_print_or_log({"to_sign_s:", to_sign_s});
    ECDSA<ECP, SHA256>::PrivateKey private_key;
    std::string signature;
    crypto.ecdsa_load_private_key_from_string(private_key);
    if (crypto.ecdsa_sign_message(private_key, to_sign_s, signature))
    {
        message_j["signature"] = crypto.base64_encode(signature);
    }

    std::string message_s = message_j.dump();

    pl.handle_print_or_log({"Hello_reward request sent"});

    Network::P2pNetwork pn;
    pn.p2p_client(ip, message_s);
}

std::string PocoCrowd::hash_of_block_ = "";
nlohmann::json PocoCrowd::block_j_ = {};