#include "p2p_network.hpp"
#include "p2p_network_c.hpp"

#include "protocol_c.hpp"

using namespace Coin;

void P2pNetworkC::handle_read_client_c(nlohmann::json buf_j, boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints)
{
    //
    Common::Print_or_log pl;
    pl.handle_print_or_log({"buf_j server", buf_j});

    std::string req = buf_j["req"];
    std::map<std::string, int> req_conversion;
    req_conversion["update_me_c"] =     100;

    switch (req_conversion[req])
    {
        case 100:   update_me_c_client(buf_j, std::ref(io_context), std::ref(endpoints));
                    break;
        case 101:   update_you_c_client(buf_j, std::ref(io_context), std::ref(endpoints));
                    break;
        default:    break;
    }
}

void P2pNetworkC::update_me_c_client(nlohmann::json buf_j, boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Update_you_c: send all blocks, rocksdb and matrices to server (client)"});

    std::string req_latest_block = buf_j["block_nr"];

    nlohmann::json msg;
    msg["req"] = "update_you_c";
    ProtocolC protoc;

    // Update blockchain
    msg["blocks"] = protoc.get_blocks_from_c(req_latest_block);

    // nlohmann::json list_of_users_j = nlohmann::json::parse(protoc.get_all_users_from_c(req_latest_block)); // TODO: there are double parse/dumps everywhere
    //                                                                                                     // maybe even a stack is better ...
    // // Update rocksdb
    // nlohmann::json rdb;
    // Rocksy* rocksy = new Rocksy("transactiondbreadonly");
    // for (auto& user : list_of_users_j)
    // {
    //     nlohmann::json usr;
    //     std::string u = user;
    //     nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(u));
    //     usr = {u: value_j};
    //     rdb.push_back(usr);
    // }
    // delete rocksy;

    // msg["rocksdb"] = rdb;

    // // Update matrices
    // Poco::BlockMatrix bm;
    // Poco::IntroMsgsMat imm;
    // Poco::IpAllHashes iah;
    // nlohmann::json contents_j;

    // for (int i = 0; i < bm.get_block_matrix().size(); i++)
    // {
    //     for (int j = 0; j < bm.get_block_matrix().at(i).size(); j++)
    //     {
    //         contents_j[std::to_string(i)][std::to_string(j)] = *bm.get_block_matrix().at(i).at(j);
    //     }
    // }
    // msg["bm"] = contents_j;
    // contents_j.clear();

    // for (int i = 0; i < imm.get_intro_msg_s_3d_mat().size(); i++)
    // {
    //     for (int j = 0; j < imm.get_intro_msg_s_3d_mat().at(i).size(); j++)
    //     {
    //         for (int k = 0; k < imm.get_intro_msg_s_3d_mat().at(i).at(j).size(); k++)
    //         {
    //             contents_j[std::to_string(i)][std::to_string(j)][std::to_string(k)] = *imm.get_intro_msg_s_3d_mat().at(i).at(j).at(k);
    //         }
    //     }
    // }
    // msg["imm"] = contents_j;
    // contents_j.clear();

    // for (int i = 0; i < iah.get_ip_all_hashes_3d_mat().size(); i++)
    // {
    //     for (int j = 0; j < iah.get_ip_all_hashes_3d_mat().at(i).size(); j++)
    //     {
    //         for (int k = 0; k < iah.get_ip_all_hashes_3d_mat().at(i).at(j).size(); k++)
    //         {
    //             contents_j[std::to_string(i)][std::to_string(j)][std::to_string(k)]["first"] = (*iah.get_ip_all_hashes_3d_mat().at(i).at(j).at(k)).first;
    //             contents_j[std::to_string(i)][std::to_string(j)][std::to_string(k)]["second"] = (*iah.get_ip_all_hashes_3d_mat().at(i).at(j).at(k)).second;
    //         }
    //     }
    // }
    // msg["iah"] = contents_j;
    // contents_j.clear();

    // // Update intro_msg_vec and ip_hemail_vec
    // msg["imv"];
    // for (auto& el: intro_msg_vec_.get_intro_msg_vec())
    // {
    //     msg["imv"].push_back(*el);
    // }

    // msg["ihv"];
    // for (auto& el: ip_hemail_vec_.get_all_ip_hemail_vec())
    // {
    //     msg["ihv"][(*el).first] = (*el).second;
    // }

    Coin::P2pClientC pcc(std::ref(io_context), std::ref(endpoints));
    pcc.set_resp_msg_client(msg.dump());
}

void P2pNetworkC::update_you_c_client(nlohmann::json buf_j, boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Update_me: receive all blocks, rocksdb and matrices from server (client)"});

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    Coin::P2pClientC pcc(std::ref(io_context), std::ref(endpoints));
    pcc.set_resp_msg_client(m_j.dump());

    // // Update blocks
    // nlohmann::json blocks_j = buf_j["blocks"];
    // for (auto& b: blocks_j.items())
    // {
    //     nlohmann::json block_j = b.value()["block"];
    //     std::string block_nr = b.value()["block_nr"];

    //     merkle_tree mt;
    //     mt.save_block_to_file(block_j, block_nr);
    // }

    // // Update rocksdb
    // nlohmann::json rdb_j = buf_j["rocksdb"];

    // for (auto& element : rdb_j)
    // {
    //     std::string key_s = element["full_hash"];
    //     std::string value_s = element.dump();

    //     Rocksy* rocksy = new Rocksy("usersdb");
    //     rocksy->Put(key_s, value_s);
    //     delete rocksy;
    // }

    // // Update matrices
    // nlohmann::json block_matrix_j = buf_j["bm"];
    // nlohmann::json intro_msg_s_matrix_j = buf_j["imm"];
    // nlohmann::json ip_all_hashes_j = buf_j["iah"];

    // Poco::BlockMatrix bm;
    // Poco::IntroMsgsMat imm;
    // Poco::IpAllHashes iah;

    // bm.get_block_matrix().clear(); // TODO clear the matrix --> this doesn't clear it
    // bm.get_calculated_hash_matrix().clear();
    // bm.get_prev_hash_matrix().clear();

    // for (auto& [k1, v1] : block_matrix_j.items())
    // {
    //     for (auto& [k2, v2] : v1.items())
    //     {
    //         bm.add_block_to_block_vector(v2);
    //         bm.add_calculated_hash_to_calculated_hash_vector(v2);
    //         bm.add_prev_hash_to_prev_hash_vector(v2);
    //     }

    //     bm.add_block_vector_to_block_matrix();
    //     bm.add_calculated_hash_vector_to_calculated_hash_matrix();
    //     bm.add_prev_hash_vector_to_prev_hash_matrix();
    // }

    // for (auto& [k1, v1] : intro_msg_s_matrix_j.items())
    // {
    //     for (auto& [k2, v2] : v1.items())
    //     {
    //         for (auto& [k3, v3] : v2.items())
    //         {
    //             imm.add_intro_msg_to_intro_msg_s_vec(v3);
    //         }

    //         imm.add_intro_msg_s_vec_to_intro_msg_s_2d_mat();
    //     }

    //     imm.add_intro_msg_s_2d_mat_to_intro_msg_s_3d_mat();
    // }

    // for (auto& [k1, v1] : ip_all_hashes_j.items())
    // {
    //     for (auto& [k2, v2] : v1.items())
    //     {
    //         for (auto& [k3, v3] : v2.items())
    //         {
    //             std::pair<std::string, std::string> myPair = std::make_pair(v3["first"], v3["second"]);
    //             std::shared_ptr<std::pair<std::string, std::string>> ptr(new std::pair<std::string, std::string> (myPair));
    //             iah.add_ip_hemail_to_ip_all_hashes_vec(ptr);
    //         }

    //         iah.add_ip_all_hashes_vec_to_ip_all_hashes_2d_mat();
    //     }

    //     iah.add_ip_all_hashes_2d_mat_to_ip_all_hashes_3d_mat();
    // }

    // // Update intro_msg_vec and ip_hemail_vec
    // nlohmann::json intro_msg_vec_j = buf_j["imv"];
    // nlohmann::json ip_hemail_vec_j = buf_j["ihv"];

    // for (auto& el: intro_msg_vec_j)
    // {
    //     intro_msg_vec_.add_to_intro_msg_vec(el);
    // }

    // for (auto& [k, v]: ip_hemail_vec_j.items())
    // {
    //     ip_hemail_vec_.add_ip_hemail_to_ip_hemail_vec(k, v);
    // }

    Coin::P2pC pc;
    pc.set_coin_update_complete(true);
}