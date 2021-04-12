#include "create_block.hpp"
#include "merkle_tree.hpp"
#include "poco.hpp"
#include "p2p.hpp"
#include "rocksy.hpp"

using namespace Crowd;

CreateBlock::CreateBlock(std::vector<nlohmann::json> &message_j_vec, std::map<enet_uint32, std::string> &all_full_hashes)
{
    message_j_vec_ = message_j_vec;

    merkle_tree mt;

    nlohmann::json m_j, entry_tx_j, entry_transactions_j, exit_tx_j, exit_transactions_j;
    nlohmann::json to_block_j;
    std::string fh_s;

    for (int i = 0; i < message_j_vec_.size(); i++)
    {
        m_j = message_j_vec_[i];

        std::string full_hash_req = m_j["full_hash_req"];

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

        Crypto crypto;
        // update rocksdb
        nlohmann::json rocksdb_j;
        rocksdb_j["version"] = "O.1";
        rocksdb_j["ip"] = m_j["ip"];
        rocksdb_j["server"] = true;
        rocksdb_j["fullnode"] = true;
        std::string email_of_req = m_j["email_of_req"];
        rocksdb_j["hash_email"] = crypto.bech32_encode_sha256(email_of_req);
        PrevHash ph;
        rocksdb_j["prev_hash"] = ph.calculate_last_prev_hash_from_blocks();
        rocksdb_j["full_hash"] = full_hash_req;
        Protocol proto;
        rocksdb_j["block_nr"] = proto.get_last_block_nr();
        rocksdb_j["ecdsa_pub_key"] = m_j["ecdsa_pub_key"];
        rocksdb_j["rsa_pub_key"] = m_j["rsa_pub_key"];
        std::string rocksdb_s = rocksdb_j.dump();

        Rocksy* rocksy = new Rocksy();
        rocksy->Put(full_hash_req, rocksdb_s);
        delete rocksy;
    }

    s_shptr_ = mt.calculate_root_hash(s_shptr_);
    std::string datetime = mt.time_now();
    std::string root_hash_data = s_shptr_->top();
    block_j_ = mt.create_block(datetime, root_hash_data, entry_transactions_j, exit_transactions_j);

    PrevHash ph;
    block_j_["prev_hash"] = ph.calculate_last_prev_hash_from_blocks();

    Protocol proto;
    std::string my_last_block_nr = proto.get_last_block_nr();

    // send hash of this block with the block contents to the co's, forget save_block_to_file
    // is the merkle tree sorted, then find the last blocks that are gathered for all the co's

    // send intro_block to co's
    Poco poco;
    poco.inform_chosen_ones(my_last_block_nr, block_j_, all_full_hashes);

    std::string block_s = mt.save_block_to_file(block_j_, my_last_block_nr);
std::cout << "--------5: " << std::endl;

    set_hash_of_new_block(block_s);
}

nlohmann::json CreateBlock::get_block_j()
{
    // TODO intro_peer doesn't know its full_hash until new_block is send
    // but the other users don't know the whereabouts of intro_peer

    return block_j_;
}

std::string CreateBlock::get_hash_of_new_block()
{
    return hash_of_block_;
}

void CreateBlock::set_hash_of_new_block(std::string block)
{
    Crypto crypto;
    hash_of_block_ = crypto.bech32_encode(block);
}
