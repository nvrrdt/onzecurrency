#include "p2p_network.hpp"

#include "print_or_log.hpp"

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>

#include "p2p_message.hpp"
#include "protocol_c.hpp"
#include "p2p_c.hpp"
#include "prev_hash_c.hpp"
#include "transactions.hpp"
#include "sharding.hpp"

#include "desktop.hpp"

using boost::asio::ip::tcp;
using namespace Crowd;
using namespace Coin;
using namespace Network;
using namespace Common;
using namespace Poco;

bool UI::Normal::goto_normal_mode_ = false;

void P2pSession::handle_read_server(p2p_message read_msg_server)
{
    Common::Print_or_log pl;
    if ( !read_msg_server.get_eom_flag()) {
        std::string str_read_msg(read_msg_server.body());
        buf_server_ += str_read_msg.substr(0, read_msg_server.get_body_length());
    } else {
        // process json message
        std::string str_read_msg(read_msg_server.body());
        buf_server_ += str_read_msg.substr(0, read_msg_server.get_body_length());
        nlohmann::json buf_j = nlohmann::json::parse(buf_server_);

        std::string req = buf_j["req"];
        std::map<std::string, int> req_conversion;
        req_conversion["register"] =            1;
        req_conversion["connect"] =             2;
        req_conversion["intro_peer"] =          3;
        req_conversion["new_peer"] =            4;
        req_conversion["intro_prel_block"] =    7;
        req_conversion["new_prel_block"] =      8;
        req_conversion["intro_final_block"] =   9;
        req_conversion["new_final_block"] =     10;
        req_conversion["your_full_hash"] =      11;
        req_conversion["hash_comparison"] =     13;
        req_conversion["intro_online"] =        16;
        req_conversion["new_online"] =          17;
        req_conversion["intro_offline"] =       18;
        req_conversion["new_offline"] =         19;
        req_conversion["update_you"] =          20;

        req_conversion["hello_tx"] =        100;
        req_conversion["intro_tx"] =        101;
        req_conversion["new_tx"] =          102;
        req_conversion["hello_reward"] =    103;
        req_conversion["intro_reward"] =    104;
        req_conversion["new_reward"] =      105;
        req_conversion["intro_block_c"] =   106;
        req_conversion["hash_comparison_c"] =   107;
        req_conversion["new_block_c"] =     108;

        switch (req_conversion[req])
        {
            case 1:     register_for_nat_traversal(buf_j);
                        break;
            case 2:     connect_to_nat(buf_j);
                        break;
            case 3:     intro_peer(buf_j);
                        break;
            case 4:     new_peer(buf_j);
                        break;
            case 7:     intro_prel_block(buf_j);
                        break;
            case 8:     new_prel_block(buf_j);
                        break;
            case 9:     intro_final_block(buf_j);
                        break;
            case 10:    new_final_block(buf_j);
                        break;
            case 11:    your_full_hash(buf_j);
                        break;
            case 13:    hash_comparison(buf_j);
                        break;
            case 16:    intro_online(buf_j);
                        break;
            case 17:    new_online(buf_j);
                        break;
            case 18:    intro_offline(buf_j);
                        break;
            case 19:    new_offline(buf_j);
                        break;
            case 20:    update_you_server(buf_j);
                        break;

            case 100:   hello_tx(buf_j);
                        break;
            case 101:   intro_tx(buf_j);
                        break;
            case 102:   new_tx(buf_j);
                        break;
            case 103:   hello_reward(buf_j);
                        break;
            case 104:   intro_reward(buf_j);
                        break;
            case 105:   new_reward(buf_j);
                        break;
            case 106:   intro_block_c(buf_j);
                        break;
            case 107:   hash_comparison_c(buf_j);
                        break;
            case 108:   new_block_c(buf_j);
                        break;

            default:    pl.handle_print_or_log({"Function not found: error server"});
                        break;
        }

        buf_server_ = "";
    }
}

void P2pSession::register_for_nat_traversal(nlohmann::json buf_j)
{
    nlohmann::json resp_j;
    resp_j["register"] = "ack";

    set_resp_msg_server(resp_j.dump());
    
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Ack for registering client is confirmed"});
}

void P2pSession::connect_to_nat(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"connection request for a peer behind a nat"});

    nlohmann::json resp_j;
    resp_j["req"] = "connect";
    resp_j["connect"] = "ok";
    resp_j["id_to"] = buf_j["id_to"];
    resp_j["ip_to"] = buf_j["ip_to"];
    resp_j["id_from"] = buf_j["id_from"];
    resp_j["ip_from"] = buf_j["ip_from"];

    set_resp_msg_server(resp_j.dump()); 
}

void P2pSession::intro_peer(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Intro_peer req recv", socket_.remote_endpoint().address().to_string()});

    // process buf_j["hash_of_req"] to find ip of the peer who should update you
    std::string co_from_req = buf_j["full_hash_co"];
    std::string email_of_req = buf_j["email_of_req"];
    std::string hash_of_email = buf_j["hash_of_email"];
    // std::string prev_hash_req = buf_j["prev_hash_of_req"];
    std::string ecdsa_pub_key_s = buf_j["ecdsa_pub_key"];
    std::string rsa_pub_key = buf_j["rsa_pub_key"];
    std::string signature = buf_j["signature"];
    std::string req_latest_block = buf_j["latest_block"];

    // close the conn when you're genesis
    Protocol* proto = new Protocol();
    std::string my_latest_block = proto->get_last_block_nr();
    if (my_latest_block == "no blockchain present in folder")
    {
        // Disconect from client
        nlohmann::json msg_j;
        msg_j["req"] = "close_same_conn";
        set_resp_msg_server(msg_j.dump());

        return;
    }

    // Validate email && verify message
    Auth auth;

    nlohmann::json to_verify_j;
    to_verify_j["ecdsa_pub_key"] = ecdsa_pub_key_s;
    to_verify_j["rsa_pub_key"] = rsa_pub_key;
    to_verify_j["email"] = email_of_req;

    Crypto* crypto = new Crypto();
    std::string to_verify_s = to_verify_j.dump();
    ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
    crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
    std::string signature_bin = crypto->base64_decode(signature);
    
    if (auth.validateEmail(email_of_req) && crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
    {
        pl.handle_print_or_log({"Email validated and message verified"});

        /** The plan for the capstone of poco:
         * - Make a vector of prev_hashes
         * - Share block_matrix().back() with first connected peer
         * - Compare prel_full_hash with prev_hash_from_block_matrix_entry == my_full_hash
         *   --> final my_full_hash must later be changed when there's a final block with you in
         * 
         * How to test this? You're probably blinded by the amount of data
         * 
         * - rocksdb can only be filled when in final block
         */

        // upload blockchain and block_matrix --> must later also be directed to a new_co, because of decentralisation

        PrevHash ph;
        std::string prel_first_prev_hash_req = ph.calculate_hash_from_last_block();

        std::string hash_of_email_prev_hash_concatenated = hash_of_email + prel_first_prev_hash_req; // TODO should this anonymization not be numbers instead of strings?
        std::string prel_first_full_hash_req =  crypto->bech32_encode_sha256(hash_of_email_prev_hash_concatenated);

        FullHash fh;
        std::string my_full_hash = fh.get_full_hash();

        pl.handle_print_or_log({"______: ", "fh =", prel_first_full_hash_req, "ph =", prel_first_prev_hash_req, email_of_req});

        Rocksy* rocksy = new Rocksy("usersdbreadonly");
        std::string prel_first_coordinator_server = rocksy->FindCoordinator(prel_first_full_hash_req);
        delete rocksy;

        pl.handle_print_or_log({"my_full_hash: ", my_full_hash});
        pl.handle_print_or_log({"prel_first_coordinator: ", prel_first_coordinator_server});

        if (my_full_hash != "" && my_full_hash == prel_first_coordinator_server)
        {
            // pl.handle_print_or_log({"My latest block:", my_latest_block});
            // pl.handle_print_or_log({"Req latest block: ", req_latest_block});

            if (req_latest_block < my_latest_block || req_latest_block == "no blockchain present in folder")
            {
                // Send first block to peer to enable datetime synchonisation
                nlohmann::json first_block_j = proto->get_block_at("0");

                nlohmann::json msg;
                msg["req"] = "send_first_block";
                msg["block"] = first_block_j;
                set_resp_msg_server(msg.dump());
            }

            // Disconect from client
            nlohmann::json msg_j;
            msg_j["req"] = "close_this_conn_and_create";
            set_resp_msg_server(msg_j.dump());

            pl.handle_print_or_log({"1 or more totalamountofpeers!"});

            // communicate intro_peers to chosen_one's with a new_peer req

            std::map<int, std::string> parts = proto->partition_in_buckets(my_full_hash, my_full_hash);

            nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
            message_j["req"] = "new_peer";
            message_j["email_of_req"] = email_of_req;
            message_j["hash_of_email"] = hash_of_email;
            message_j["full_hash_co"] = my_full_hash;
            message_j["ecdsa_pub_key"] = ecdsa_pub_key_s;
            message_j["rsa_pub_key"] = rsa_pub_key;
            message_j["ip"] = socket_.remote_endpoint().address().to_string();

            to_sign_j["ecdsa_pub_key"] = ecdsa_pub_key_s;
            to_sign_j["rsa_pub_key"] = rsa_pub_key;
            to_sign_j["email_of_req"] = email_of_req;
            std::string to_sign_s = to_sign_j.dump();
            ECDSA<ECP, SHA256>::PrivateKey private_key;
            std::string signature;
            crypto->ecdsa_load_private_key_from_string(private_key);
            if (crypto->ecdsa_sign_message(private_key, to_sign_s, signature))
            {
                message_j["signature"] = crypto->base64_encode(signature);
            }

            Poco::BlockMatrix bm;
            for (int i = 1; i <= parts.size(); i++)
            {
                if (i == 1) continue; // ugly hack for a problem in proto.partition_in_buckets()
                if (parts[i] == "") continue; // UGLY hack: "" should be "0"

                Rocksy* rocksy1 = new Rocksy("usersdbreadonly");

                // lookup in rocksdb
                std::string val = parts[i];
                nlohmann::json value_j = nlohmann::json::parse(rocksy1->Get(val));
                std::string ip = value_j["ip"];

                delete rocksy1;

                // if peer ip == this server's ip --> send new_peer to kids
                // --> from_to(my_hash, my_hash) if just me then connected_peers + from_to(my_hash, next hash)
                // --> if more then t.client to same layer co
                // in bucket --> poco --> coord connects to all co --> co connect to other co --> communicate final_hash --> register amount of ok's and nok's

                P2pNetwork pn;
                // inform the underlying network
                pl.handle_print_or_log({"Send new_peer req: Inform my underlying network as coordinator"});

                std::string next_hash;
                if (i != parts.size())
                {
                    next_hash = parts[i+1];
                }
                else
                {
                    next_hash = parts[1];
                }
                
                std::map<int, std::string> parts_underlying = proto->partition_in_buckets(my_full_hash, next_hash);
                std::string key2, val2;
                Rocksy* rocksy2 = new Rocksy("usersdbreadonly");
                for (int i = 1; i <= parts_underlying.size(); i++)
                {
                    if (i == 1) continue; // ugly hack for a problem in proto.partition_in_buckets()
                    if (parts_underlying[i] == my_full_hash) continue;

                    // lookup in rocksdb
                    std::string val2 = parts_underlying[i];
                    nlohmann::json value_j = nlohmann::json::parse(rocksy2->Get(val2));
                    std::string ip_underlying = value_j["ip"];

                    pl.handle_print_or_log({"Send new_peer req: Non-connected underlying peers - client: ", ip_underlying});

                    Poco::PocoCrowd pc;
                    bool cont = false;
                    for (auto& el: pc.get_new_users_ip())
                    {
                        if (el == ip_underlying)
                        {
                            cont = true;
                            break;
                        }
                    }
                    if (cont) continue;

                    // message to non-connected peers
                    std::string message = message_j.dump();
                    pn.p2p_client(ip_underlying, message);
                }
                delete rocksy2;

                // inform the other peer's in the same layer (as coordinator)
                pl.handle_print_or_log({"Send new_peer req: Inform my equal layer as coordinator: ", ip});
                
                Poco::PocoCrowd pc;
                bool cont = false;
                for (auto& el: pc.get_new_users_ip())
                {
                    if (el == ip)
                    {
                        cont = true;
                        break;
                    }
                }
                if (cont) continue;

                std::string message = message_j.dump();
                pn.p2p_client(ip, message);
            }

            // wait 20 seconds or > 1 MB to create block, to process the timestamp if you are the first new_peer request
            intro_msg_vec_.add_to_intro_msg_vec(message_j);

            ip_hemail_vec_.add_ip_hemail_to_ip_hemail_vec(message_j["ip"], hash_of_email); // TODO you have to reset this
        }
        else
        {
            // There's another chosen_one, reply with the correct chosen_one
            pl.handle_print_or_log({"New_co: Chosen_one is someone else!"});

            // find shard and put its ip's in an json array:
            // calculate_shard_nr and its limits
            // search these limits in rocksdb and put in json array

            nlohmann::json message_j;
            message_j["req"] = "new_co";

            Poco::DatabaseSharding ds;
            auto shard_users = ds.get_shard_users(prel_first_coordinator_server);
            Rocksy* rocksy = new Rocksy("usersdbreadonly");
            nlohmann::json peer_ips;
            for (auto& user: shard_users)
            {
                nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(user));
                peer_ips.push_back(value_j["ip"]);
            }
            delete rocksy;

            message_j["ips_shard"] = peer_ips;
            set_resp_msg_server(message_j.dump());
        }
        
        delete crypto;
    }
    else
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Email incorrect and/or message verification incorrect"});
    }

    delete proto;
    
    // verify message, lookup peer in rocksdb and verify that you are the chose_one,
    // if not exists in rocksdb continue sending new_peer to all, if exist respond with an 'user_exists'



    // // get ip from ip_peers.json // TODO: put this in p2p.hpp, it's a copy
    // IpPeers ip;
    // std::vector<std::string> ip_s = ip.get_ip_s();
    // nlohmann::json json;
    // P2p p;
    // p.to_json(json, ip_s);
    // std::cout << "ip_s_server: " << json["ip_list"] << std::endl;

    // const std::string ip_mother_peer = json["ip_list"][0]; // TODO: ip should later be randomly taken from rocksdb and/or a pre-defined list

    // if (json["ip_list"].size() == 1) // 1 ip == ip_mother_peer
    // {
    //     // 1) Wait 30 seconds or till 1 MB of "intro_peer"'s is reached and then to create a block
    //     // 2) If ok: create block with final hash
    //     // 3) then: update the network with room_.deliver(msg)
    //     // 4) add peer to ip_list

    //     Poco::PocoCoin poco(email_of_peer, hash_of_peer); // moet bij new_peer
    //     std::cout << "Is this reached? " << hash_of_peer << std::endl;
    //     // if poco ok: update blockchain and update rocksdb will be received through the chosen one's
    // }
    // else
    // {
    //     // If there are more peers in the ip_list ...
    // }
}

void P2pSession::new_peer(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"New_peer req recv:"});
    // should read the timestamp of the first new_peer request received
    
    // wait 20 seconds or > 1 MB to create block, to process the timestamp if you are the first new_peer request
    intro_msg_vec_.add_to_intro_msg_vec(buf_j);

    ip_hemail_vec_.add_ip_hemail_to_ip_hemail_vec(buf_j["ip"], buf_j["hash_of_email"]); // TODO you have to reset this

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());
}

void P2pSession::intro_prel_block(nlohmann::json buf_j)
{
    // intro_prel_block
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Intro_prel_block:"});

    // communicate preliminary block to other peers --> needs to be calculated depending on this server's place in the chosen_ones list

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());

    // add received_blocks to received_block_vector
    nlohmann::json recv_block_j = buf_j["block"];
    Poco::BlockMatrix *bm = new Poco::BlockMatrix();
    bm->add_received_block_to_received_block_vector(recv_block_j);

    // // for debugging purposes:
    // for (int i = 0; i < bm->get_received_block_matrix().size(); i++)
    // {
    //     for (int j = 0; j < bm->get_received_block_matrix().at(i).size(); j++)
    //     {
    //         nlohmann::json content_j = *bm->get_received_block_matrix().at(i).at(j);
    //         std::cout << "received block matrix entry prel " << i << " " << j << " (oldest first)" << std::endl << std::endl;
    //     }
    // }
    // // for debugging purposes:
    // for (int i = 0; i < bm->get_block_matrix().size(); i++)
    // {
    //     for (int j = 0; j < bm->get_block_matrix().at(i).size(); j++)
    //     {
    //         nlohmann::json content_j = *bm->get_block_matrix().at(i).at(j);
    //         std::cout << "block matrix entry prel" << i << " " << j << " (oldest first)" << std::endl;
    //     }
    // }

    // Is the coordinator the truthful real coordinator for this block
    std::string full_hash_coord = buf_j["full_hash_coord"];

    FullHash fh;
    std::string my_full_hash = fh.get_full_hash();

    nlohmann::json message_j;

    nlohmann::json chosen_ones = buf_j["chosen_ones"];
    bool is_chosen_one  = false;
    for (auto& el: chosen_ones.items())
    {
        if (el.value() == my_full_hash) is_chosen_one = true;
    }

    if (is_chosen_one) // full_hash_coord should be one of the chosen_ones
    {
        pl.handle_print_or_log({"Intro prel chosen_one is truthful"});

        // p2p_client() to all calculated other chosen_ones

        std::string next_full_hash;
        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (chosen_ones[i] == my_full_hash)
            {
                if (i != chosen_ones.size() - 1)
                {
                    next_full_hash = chosen_ones[i+1];
                }
                else
                {
                    next_full_hash = chosen_ones[0];
                }
            }
        }

        Protocol proto;
        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, next_full_hash);

        nlohmann::json to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "new_prel_block";
        message_j["latest_block_nr"] = buf_j["latest_block_nr"];
        message_j["block"] = buf_j["block"];
        message_j["prev_hash"] = buf_j["prev_hash"];
        message_j["full_hash_coord"] = buf_j["full_hash_coord"];

        int k;
        std::string v;
        for (auto &[k, v] : parts)
        {
            message_j["chosen_ones"].push_back(v);
        }

        to_sign_j["latest_block_nr"] = message_j["latest_block_nr"];
        to_sign_j["block"] = message_j["block"];
        to_sign_j["prev_hash"] = message_j["prev_hash"];
        to_sign_j["full_hash_coord"] = message_j["full_hash_coord"];
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
        Poco::Synchronisation sync;
        int key;
        std::string val;
        for (auto &[key, val] : parts)
        {
            if (key == 1) continue;
            // if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
            // if (val == parts[1]) continue; // TODO --> UGLY --> somehow the first and the last chosen_one are the same, you don't need both

            Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");

            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
            std::string peer_ip = value_j["ip"];

            delete rocksy;
            
            std::string message = message_j.dump();

            pl.handle_print_or_log({"Preparation for new_prel_block: ", peer_ip});

            Poco::PocoCrowd pc;
            bool cont = false;
            for (auto& el: pc.get_new_users_ip())
            {
                if (el == peer_ip)
                {
                    cont = true;
                    break;
                }
            }
            if (cont) continue;


            // p2p_client() to all chosen ones with new_peer request
            pn.p2p_client(peer_ip, message);
        }

        delete bm;
    }
    else
    {
        pl.handle_print_or_log({"Intro prel chosen_one is not truthful"});
    }
}

void P2pSession::new_prel_block(nlohmann::json buf_j)
{
    // new_prel_block --> TODO block should be saved
    Common::Print_or_log pl;
    pl.handle_print_or_log({"New_prel_block:"});

    // communicate preliminary block to other peers --> needs to be calculated depending on this server's place in the chosen_ones list

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());

    // add received_blocks to received_block_vector
    nlohmann::json recv_block_j = buf_j["block"];
    Poco::BlockMatrix *bm = new Poco::BlockMatrix();
    bm->add_received_block_to_received_block_vector(recv_block_j);

    // // for debugging purposes:
    // for (int i = 0; i < bm->get_received_block_matrix().size(); i++)
    // {
    //     for (int j = 0; j < bm->get_received_block_matrix().at(i).size(); j++)
    //     {
    //         nlohmann::json content_j = *bm->get_received_block_matrix().at(i).at(j);
    //         std::cout << "received block matrix entry prel " << i << " " << j << " (oldest first)" << std::endl << std::endl;
    //     }
    // }
    // // for debugging purposes:
    // for (int i = 0; i < bm->get_block_matrix().size(); i++)
    // {
    //     for (int j = 0; j < bm->get_block_matrix().at(i).size(); j++)
    //     {
    //         nlohmann::json content_j = *bm->get_block_matrix().at(i).at(j);
    //         std::cout << "block matrix entry prel" << i << " " << j << " (oldest first)" << std::endl;
    //     }
    // }

    // Is the coordinator the truthful real coordinator for this block
    std::string full_hash_coord = buf_j["full_hash_coord"];

    FullHash fh;
    std::string my_full_hash = fh.get_full_hash();

    nlohmann::json message_j;

    nlohmann::json chosen_ones = buf_j["chosen_ones"];
    bool is_chosen_one  = false;
    for (auto& el: chosen_ones.items())
    {
        if (el.value() == my_full_hash) is_chosen_one = true;
    }

    if (is_chosen_one)
    {
        pl.handle_print_or_log({"New prel chosen_one is truthful"});

        // p2p_client() to all calculated other chosen_ones

        std::string next_full_hash;
        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (chosen_ones[i] == my_full_hash)
            {
                if (i != chosen_ones.size() - 1)
                {
                    next_full_hash = chosen_ones[i];
                }
                else
                {
                    next_full_hash = chosen_ones[0];
                }
            }
        }

        Protocol proto;
        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, next_full_hash);

        nlohmann::json to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "new_prel_block";
        message_j["latest_block_nr"] = buf_j["latest_block_nr"];
        message_j["block"] = buf_j["block"];
        message_j["prev_hash"] = buf_j["prev_hash"];
        message_j["full_hash_coord"] = buf_j["full_hash_coord"];

        int k;
        std::string v;
        for (auto &[k, v] : parts)
        {
            message_j["chosen_ones"].push_back(v);
        }

        to_sign_j["latest_block_nr"] = message_j["latest_block_nr"];
        to_sign_j["block"] = message_j["block"];
        to_sign_j["prev_hash"] = message_j["prev_hash"];
        to_sign_j["full_hash_coord"] = message_j["full_hash_coord"];
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
        Poco::Synchronisation sync;
        int key;
        std::string val;
        for (auto &[key, val] : parts)
        {
            if (key == 1) continue;
            // if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
            // if (val == full_hash_coord) continue;
            // if (val == parts[1]) continue; // TODO --> UGLY --> somehow the first and the last chosen_one are the same, you don't need both
            
            Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");

            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
            std::string peer_ip = value_j["ip"];

            delete rocksy;
            
            std::string message = message_j.dump();

            pl.handle_print_or_log({"Prepaation for secondary new_prel_block: ", peer_ip});

            Poco::PocoCrowd pc;
            bool cont = false;
            for (auto& el: pc.get_new_users_ip())
            {
                if (el == peer_ip)
                {
                    cont = true;
                    break;
                }
            }
            if (cont) continue;

            // p2p_client() to all chosen ones with intro_peer request
            pn.p2p_client(peer_ip, message);
        }

        delete bm;
pl.handle_print_or_log({"___10 end of new_prel_block"});
    }
    else
    {
        pl.handle_print_or_log({"New prel chosen_one is not truthful"});
    }
}

void P2pSession::intro_final_block(nlohmann::json buf_j)
{
    // intro_final_block
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Intro_final_block:"});

    // Is the coordinator the truthful final coordinator for this block?
    // Compare the hashes from the block of the coordinator with your saved blocks' hashes
    // communicate comparison to other chosen_ones --> needs to be calculated depending on this server's place in the chosen_ones list
    // Then inform your underlying network

    nlohmann::json recv_block_j = buf_j["block"];
    std::string recv_latest_block_nr_s = buf_j["latest_block_nr"];
    std::string full_hash_coord_from_coord = buf_j["full_hash_coord"];
    nlohmann::json chosen_ones = buf_j["chosen_ones"];
    nlohmann::json rocksdb_j = buf_j["rocksdb"];

    FullHash fh;
    std::string my_full_hash = fh.get_full_hash();

    std::string recv_block_s = recv_block_j.dump();
    Common::Crypto crypto;
    std::string prev_hash_me = crypto.bech32_encode_sha256(recv_block_s);
    Rocksy* rocksy = new Rocksy("usersdbreadonly");
    std::string full_hash_coord_from_me = rocksy->FindCoordinator(prev_hash_me);

    delete rocksy;


    if (full_hash_coord_from_me == full_hash_coord_from_coord)
    {
        pl.handle_print_or_log({"Coordinator is truthful"});

        // Save block
        merkle_tree mt;
        mt.save_block_to_file(recv_block_j, recv_latest_block_nr_s); // TODO what about parallel blocks?

        // Fill rocksdb
        for (auto& [k, v]: rocksdb_j.items())
        {
            nlohmann::json val_j = v;
            std::string key_s = val_j["full_hash"];
            std::string value_s = val_j.dump();

            Rocksy* rocksy2 = new Rocksy("usersdb");
            rocksy2->Put(key_s, value_s);
            delete rocksy2;
        }

        Protocol proto;
        nlohmann::json saved_block_at_place_i = proto.get_block_at(recv_latest_block_nr_s);
        std::string saved_block_at_place_i_s = saved_block_at_place_i.dump();
        std::string prev_hash_from_saved_block_at_place_i = crypto.bech32_encode_sha256(saved_block_at_place_i_s);

        bool comparison = prev_hash_me == prev_hash_from_saved_block_at_place_i;
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Comparison is: ", std::to_string(comparison)});

        // Inform coordinator of succesfullness of hash comparison
        nlohmann::json m_j;
        m_j["req"] = "hash_comparison";
        m_j["hash_comp"] = prev_hash_me == prev_hash_from_saved_block_at_place_i;
        std::string msg_s = m_j.dump();
        set_resp_msg_server(msg_s);

        // p2p_client() to all calculated other chosen_ones
        // this is in fact the start of the consensus algorithm where a probability is calculated
        // you don't need full consensus in order to create a succesful block
        // but full consensus improves your chances of course greatly

        int j;

        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (chosen_ones[i] == buf_j["full_hash_coord"])
            {
                j = i;
            }
        }

        P2pNetwork pn;
        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (i < j)
            {
                if (chosen_ones[i] == buf_j["full_hash_coord"]) continue;

                std::string c_one = chosen_ones[i];

                Rocksy* rocksy3 = new Rocksy("usersdbreadonly");
                nlohmann::json value_j = nlohmann::json::parse(rocksy3->Get(c_one));
                delete rocksy3;

                std::string ip = value_j["ip"];

                nlohmann::json msg_j;
                msg_j["req"] = "hash_comparison";
                msg_j["hash_comp"] = prev_hash_me == prev_hash_from_saved_block_at_place_i;
                std::string msg_s = msg_j.dump();

                pn.p2p_client(ip, msg_s);
            }
            else if (i == j)
            {
                continue;
            }
        }

        // inform your underlying ones from this block
        std::string next_full_hash;
        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (chosen_ones[i] == my_full_hash)
            {
                if (chosen_ones.size() > 1 && i < chosen_ones.size() - 1)
                {
                    next_full_hash = chosen_ones[i+1];
                }
                else
                {
                    next_full_hash = chosen_ones[0];
                }
            }
        }

        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, next_full_hash);

        nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "new_final_block";
        message_j["latest_block_nr"] = buf_j["latest_block_nr"];
        message_j["block"] = buf_j["block"];
        message_j["full_hash_coord"] = buf_j["full_hash_coord"];
        message_j["rocksdb"] = buf_j["rocksdb"];

        int k;
        std::string v;
        for (auto &[k, v] : parts)
        {
            message_j["chosen_ones"].push_back(v);
        }

        to_sign_j["latest_block_nr"] = message_j["latest_block_nr"];
        to_sign_j["block"] = message_j["block"];
        to_sign_j["full_hash_coord"] = message_j["full_hash_coord"];
        to_sign_j["rocksdb"] = message_j["rocksdb"];
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

        int key;
        std::string val;
        Poco::BlockMatrix bm;
        for (auto &[key, val] : parts)
        {
            if (key == 1) continue;
            if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
            if (val == full_hash_coord_from_coord) continue;
            if (val == parts[1]) continue; // TODO --> UGLY --> somehow the first and the last chosen_one are the same, you don't need both

            Crowd::Rocksy* rocksy4 = new Crowd::Rocksy("usersdbreadonly");

            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy4->Get(val));
            std::string peer_ip = value_j["ip"];

            delete rocksy4;
            
            std::string message = message_j.dump();

            pl.handle_print_or_log({"Preparation for new_final_block:", peer_ip});

            Poco::PocoCrowd pc;
            bool cont = false;
            for (auto& el: pc.get_new_users_ip())
            {
                if (el == peer_ip)
                {
                    cont = true;
                    break;
                }
            }
            if (cont) continue;

            // p2p_client() to all chosen ones with intro_peer request
            pn.p2p_client(peer_ip, message);
        }
    }
    else
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Final coordinator is not truthful"});

        // Disconect from client
        nlohmann::json m_j;
        m_j["req"] = "close_this_conn";
        set_resp_msg_server(m_j.dump());
    }
}

void P2pSession::new_final_block(nlohmann::json buf_j)
{
    // new_final_block
    Common::Print_or_log pl;
    pl.handle_print_or_log({"New_final_block:"});

    // Is the coordinator the truthful final coordinator for this block?
    // Compare the hashes from the block of the coordinator with your saved blocks' hashes
    // communicate comparison to other chosen_ones --> needs to be calculated depending on this server's place in the chosen_ones list
    // Then inform your underlying network

    nlohmann::json recv_block_j = buf_j["block"];
    std::string recv_latest_block_nr_s = buf_j["latest_block_nr"];
    std::string full_hash_coord_from_coord = buf_j["full_hash_coord"];
    nlohmann::json chosen_ones = buf_j["chosen_ones"];
    nlohmann::json rocksdb_j = buf_j["rocksdb"];

    // Save block
    merkle_tree mt;
    mt.save_block_to_file(recv_block_j,recv_latest_block_nr_s);

    // Fill rocksdb
    for (auto& [k, v]: rocksdb_j.items())
    {
        nlohmann::json val_j = v;
        std::string key_s = val_j["full_hash"];
        std::string value_s = val_j.dump();

        Rocksy* rocksy = new Rocksy("usersdb");
        rocksy->Put(key_s, value_s);
        delete rocksy;
    }
    
    FullHash fh;
    std::string my_full_hash = fh.get_full_hash();

    std::string recv_block_s = recv_block_j.dump();
    Common::Crypto crypto;
    std::string prev_hash_me = crypto.bech32_encode_sha256(recv_block_s);

    Protocol proto;
    std::string prev_hash_from_saved_block_at_place_i = proto.get_block_at(recv_latest_block_nr_s);

    bool comparison = prev_hash_me == prev_hash_from_saved_block_at_place_i;
    pl.handle_print_or_log({"New comparison is:", std::to_string(comparison)});

    // Inform coordinator of succesfullness of hash comparison
    nlohmann::json mm_j;
    mm_j["req"] = "hash_comparison";
    mm_j["hash_comp"] = prev_hash_me == prev_hash_from_saved_block_at_place_i;
    std::string msg_s = mm_j.dump();

    set_resp_msg_server(msg_s);

    // p2p_client() to all calculated other chosen_ones
    // this is in fact the start of the consensus algorithm where a probability is calculated
    // you don't need full consensus in order to create a succesful block
    // but full consensus improves your chances of course greatly

    int j;

    for (int i = 0; i < chosen_ones.size(); i++)
    {
        if (chosen_ones[i] == buf_j["full_hash_coord"])
        {
            j = i;
        }
    }

    P2pNetwork pn;
    for (int i = 0; i < chosen_ones.size(); i++)
    {
        if (i < j)
        {
            if (chosen_ones[i] == buf_j["full_hash_coord"]) continue;

            std::string c_one = chosen_ones[i];
            Rocksy* rocksy2 = new Rocksy("usersdbreadonly");
            nlohmann::json value_j = nlohmann::json::parse(rocksy2->Get(c_one));
            delete rocksy2;

            std::string ip = value_j["ip"];

            nlohmann::json msg_j;
            msg_j["req"] = "hash_comparison";
            msg_j["hash_comp"] = prev_hash_me == prev_hash_from_saved_block_at_place_i;
            std::string msg_s = msg_j.dump();

            pn.p2p_client(ip, msg_s);
        }
        else if (i == j)
        {
            continue;
        }
    }

    // inform your underlying ones from this block
    std::string next_full_hash;
    for (int i = 0; i < chosen_ones.size(); i++)
    {
        if (chosen_ones[i] == my_full_hash)
        {
            if (chosen_ones.size() > 1 && i < chosen_ones.size() - 1)
            {
                next_full_hash = chosen_ones[i+1];
            }
            else
            {
                next_full_hash = chosen_ones[0];
            }
        }
    }

    std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, next_full_hash);

    nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
    message_j["req"] = "new_final_block";
    message_j["latest_block_nr"] = buf_j["latest_block_nr"];
    message_j["block"] = buf_j["block"];
    message_j["full_hash_coord"] = buf_j["full_hash_coord"];
    message_j["rocksdb"] = buf_j["rocksdb"];

    int k;
    std::string v;
    for (auto &[k, v] : parts)
    {
        message_j["chosen_ones"].push_back(v);
    }

    to_sign_j["latest_block_nr"] = message_j["latest_block_nr"];
    to_sign_j["block"] = message_j["block"];
    to_sign_j["full_hash_coord"] = message_j["full_hash_coord"];
    to_sign_j["rocksdb"] = message_j["rocksdb"];
    to_sign_j["chosen_ones"] = message_j["chosen_ones"];
    // to_sign_j["rocksdb"] = message_j["rocksdb"];
    std::string to_sign_s = to_sign_j.dump();
    ECDSA<ECP, SHA256>::PrivateKey private_key;
    std::string signature;
    crypto.ecdsa_load_private_key_from_string(private_key);
    if (crypto.ecdsa_sign_message(private_key, to_sign_s, signature))
    {
        message_j["signature"] = crypto.base64_encode(signature);
    }

    int key;
    std::string val;
    Poco::BlockMatrix bm;
    for (auto &[key, val] : parts)
    {
        if (key == 1) continue;
        if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
        if (val == full_hash_coord_from_coord) continue;
        if (val == parts[1]) continue; // TODO --> UGLY --> somehow the first and the last chosen_one are the same, you don't need both

        Crowd::Rocksy* rocksy3 = new Crowd::Rocksy("usersdbreadonly");

        // lookup in rocksdb
        nlohmann::json value_j = nlohmann::json::parse(rocksy3->Get(val));
        std::string peer_ip = value_j["ip"];

        delete rocksy3;
        
        std::string message = message_j.dump();

        pl.handle_print_or_log({"Preparation for new_final_block:", peer_ip});

        Poco::PocoCrowd pc;
        bool cont = false;
        for (auto& el: pc.get_new_users_ip())
        {
            if (el == peer_ip)
            {
                cont = true;
                break;
            }
        }
        if (cont) continue;

        // p2p_client() to all chosen ones with intro_peer request
        pn.p2p_client(peer_ip, message);
    }
}

void P2pSession::your_full_hash(nlohmann::json buf_j)
{
    // my full hash
    std::string full_hash = buf_j["full_hash"];
    std::string prev_hash = buf_j["prev_hash"];
    Common::Print_or_log pl;
    pl.handle_print_or_log({"New peer's full_hash (server):", full_hash});
    pl.handle_print_or_log({"New peer's prev_hash (server):", prev_hash});

    // save full_hash
    FullHash fh;
    fh.save_full_hash(full_hash);

    // save prev_hash
    PrevHash ph;
    ph.save_my_prev_hash_to_file(prev_hash);
    
    nlohmann::json block_j = buf_j["block"];
    std::string req_latest_block_nr = buf_j["block_nr"];

    // Update my blocks, rocksdb and matrices of crowd and coin
    Protocol proto;
    std::string my_latest_block = proto.get_last_block_nr();
    ProtocolC protoc;
    std::string my_latest_block_c = protoc.get_last_block_nr_c();
    nlohmann::json m_j;
    m_j["req"] = "update_me";
    m_j["crowd"]["block_nr"] = my_latest_block;
    m_j["coin"]["block_nr"] = my_latest_block_c;
    set_resp_msg_server(m_j.dump());
}

void P2pSession::hash_comparison(nlohmann::json buf_j)
{
    // compare the received hash
    Common::Print_or_log pl;
    pl.handle_print_or_log({"The hash comparison is (server): ",  (buf_j["hash_comp"]).dump()});

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());
}

void P2pSession::intro_online(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"intro peer online: ", buf_j["full_hash"]});
    
    nlohmann::json to_verify_j;
    to_verify_j["req"] = buf_j["req"];
    to_verify_j["full_hash"] = buf_j["full_hash"];
    to_verify_j["crowd"]["block_nr"] = buf_j["crowd"]["block_nr"];
    to_verify_j["coin"]["block_nr"] = buf_j["coin"]["block_nr"];
    to_verify_j["server"] = buf_j["server"];
    to_verify_j["fullnode"] = buf_j["fullnode"];

    Rocksy* rocksy1 = new Rocksy("usersdbreadonly");
    std::string full_hash = buf_j["full_hash"];
    nlohmann::json contents_j = nlohmann::json::parse(rocksy1->Get(full_hash));
    std::string ecdsa_pub_key_s = contents_j["ecdsa_pub_key"];
    delete rocksy1;

    Crypto* crypto = new Crypto();
    std::string to_verify_s = to_verify_j.dump();
    ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
    crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
    std::string signature = buf_j["signature"];
    std::string signature_bin = crypto->base64_decode(signature);
    
    std::string my_full_hash;
    if (crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
    {
        pl.handle_print_or_log({"verified intro online user"});

        PrevHash ph;
        std::string next_prev_hash = ph.calculate_hash_from_last_block();

        std::string msg_and_nph = buf_j.dump() + next_prev_hash;
        std::string hash_msg_and_nph =  crypto->bech32_encode_sha256(msg_and_nph);

        FullHash fh;
        my_full_hash = fh.get_full_hash();

        Rocksy* rocksy2 = new Rocksy("usersdbreadonly");
        std::string coordinator_from_hash = rocksy2->FindCoordinator(hash_msg_and_nph);
        delete rocksy2;

        pl.handle_print_or_log({"my_full_hash: ", my_full_hash});
        pl.handle_print_or_log({"coordinator_from_hash: ", coordinator_from_hash});

        if (my_full_hash == coordinator_from_hash)
        {
            pl.handle_print_or_log({"I'm the intro online coordinator"});

            // inform the network of new online
            Protocol proto;

            std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

            nlohmann::json message_j, to_sign_j;
            message_j["req"] = "new_online";
            message_j["full_hash"] = full_hash;
            message_j["ip"] = socket_.remote_endpoint().address().to_string();
            message_j["server"] = buf_j["server"];
            message_j["fullnode"] = buf_j["fullnode"];
            message_j["chosen_one"] = my_full_hash;

            int k;
            std::string v;
            for (auto &[k, v] : parts)
            {
                message_j["chosen_ones"].push_back(v);
            }

            to_sign_j["req"] = message_j["req"];
            to_sign_j["full_hash"] = full_hash;
            to_sign_j["ip"] = message_j["ip"];
            to_sign_j["server"] = buf_j["server"];
            to_sign_j["fullnode"] = buf_j["fullnode"];
            to_sign_j["chosen_one"] = message_j["chosen_one"];
            to_sign_j["chosen_ones"] = message_j["chosen_ones"];
            std::string to_sign_s = to_sign_j.dump();

            Common::Crypto crypto;
            ECDSA<ECP, SHA256>::PrivateKey private_key;
            std::string signature;
            crypto.ecdsa_load_private_key_from_string(private_key);
            if (crypto.ecdsa_sign_message(private_key, to_sign_s, signature))
            {
                message_j["signature"] = crypto.base64_encode(signature);
            }
            std::string message_s = message_j.dump();

            int key;
            std::string val;
            Poco::BlockMatrix bm;
            P2pNetwork pn;
            Rocksy* rocksy3 = new Rocksy("usersdbreadonly");
            for (int i = 1; i <= parts.size(); i++)
            {
                if (i == 1) continue;

                // lookup in rocksdb
                nlohmann::json value_j = nlohmann::json::parse(rocksy3->Get(parts[i]));
                std::string peer_ip = value_j["ip"];

                // inform the underlying network
                if (parts[i] == my_full_hash) // TODO the else part isn't activated, dunno why, search in test terminals for new_peer
                {
                    // inform server's underlying network
                    pl.handle_print_or_log({"Send intro_online req: Inform my underlying network as coordinator"});

                    std::string next_hash;
                    if (i != parts.size())
                    {
                        next_hash = parts[i+1];
                    }
                    else
                    {
                        next_hash = parts[1];
                    }
                    
                    std::map<int, std::string> parts_underlying = proto.partition_in_buckets(my_full_hash, next_hash);
                    std::string key2, val2;
                    for (int i = 1; i <= parts_underlying.size(); i++)
                    {
                        if (i == 1) continue; // ugly hack for a problem in proto.partition_in_buckets()
                        if (parts_underlying[i] == my_full_hash) continue;

                        // lookup in rocksdb
                        std::string val2 = parts_underlying[i];
                        nlohmann::json value_j = nlohmann::json::parse(rocksy3->Get(val2));
                        std::string ip_underlying = value_j["ip"];

                        pl.handle_print_or_log({"Send intro_online req: Non-connected underlying peers - client: ", ip_underlying});

                        Poco::PocoCrowd pc;
                        bool cont = false;
                        for (auto& el: pc.get_new_users_ip())
                        {
                            if (el == ip_underlying)
                            {
                                cont = true;
                                break;
                            }
                        }
                        if (cont) continue;

                        // message to non-connected peers
                        std::string message = message_j.dump();
                        pn.p2p_client(ip_underlying, message);
                    }
                }
                
                pl.handle_print_or_log({"Preparation for new_online:", peer_ip});

                Poco::PocoCrowd pc;
                bool cont = false;
                for (auto& el: pc.get_new_users_ip())
                {
                    if (el == peer_ip)
                    {
                        cont = true;
                        break;
                    }
                }
                if (cont) continue;

                // p2p_client() to all chosen ones
                pn.p2p_client(peer_ip, message_s);
            }

            delete rocksy3;

            // update this rocksdb
            Rocksy* rocksy4 = new Rocksy("usersdb");
            nlohmann::json value_j = nlohmann::json::parse(rocksy4->Get(full_hash));
            value_j["online"] = true;
            value_j["ip"] = socket_.remote_endpoint().address().to_string();
            value_j["server"] = buf_j["server"];
            value_j["fullnode"] = buf_j["fullnode"];
            std::string value_s = value_j.dump();
            rocksy4->Put(full_hash, value_s);
            delete rocksy4;

            ///////
            // Update crowd
            ///////
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Update_you_crowd: send all blocks, rocksdb and matrices to server (server)"});

            std::string req_latest_block_crowd = buf_j["crowd"]["block_nr"];

            nlohmann::json msg;
            msg["req"] = "update_you";

            // Update blockchain
            msg["crowd"]["blocks"] = proto.get_blocks_from(req_latest_block_crowd);

            nlohmann::json list_of_users_j_crowd = nlohmann::json::parse(proto.get_all_users_from(req_latest_block_crowd));
            // TODO: there are double nlohmann::json::parse/dumps everywhere
            // maybe even a stack is better ...
            
            // Update rocksdb
            nlohmann::json rdb;
            Rocksy* rocksy5 = new Rocksy("usersdbreadonly");
            for (auto& user : list_of_users_j_crowd)
            {
                nlohmann::json usr;
                std::string u = user;
                nlohmann::json value_j = nlohmann::json::parse(rocksy5->Get(u));
                usr = {u: value_j};
                rdb.push_back(usr);
            }
            delete rocksy5;

            msg["crowd"]["rocksdb"] = rdb;

            // Update matrices
            Poco::IntroMsgsMat imm;
            Poco::IpAllHashes iah;
            nlohmann::json contents_j;

            for (int i = 0; i < bm.get_block_matrix().size(); i++)
            {
                for (int j = 0; j < bm.get_block_matrix().at(i).size(); j++)
                {
                    contents_j[std::to_string(i)][std::to_string(j)] = *bm.get_block_matrix().at(i).at(j);
                }
            }
            msg["crowd"]["bm"] = contents_j;
            contents_j.clear();

            for (int i = 0; i < imm.get_intro_msg_s_3d_mat().size(); i++)
            {
                for (int j = 0; j < imm.get_intro_msg_s_3d_mat().at(i).size(); j++)
                {
                    for (int k = 0; k < imm.get_intro_msg_s_3d_mat().at(i).at(j).size(); k++)
                    {
                        contents_j[std::to_string(i)][std::to_string(j)][std::to_string(k)] = *imm.get_intro_msg_s_3d_mat().at(i).at(j).at(k);
                    }
                }
            }
            msg["crowd"]["imm"] = contents_j;
            contents_j.clear();

            for (int i = 0; i < iah.get_ip_all_hashes_3d_mat().size(); i++)
            {
                for (int j = 0; j < iah.get_ip_all_hashes_3d_mat().at(i).size(); j++)
                {
                    for (int k = 0; k < iah.get_ip_all_hashes_3d_mat().at(i).at(j).size(); k++)
                    {
                        contents_j[std::to_string(i)][std::to_string(j)][std::to_string(k)]["first"] = (*iah.get_ip_all_hashes_3d_mat().at(i).at(j).at(k)).first;
                        contents_j[std::to_string(i)][std::to_string(j)][std::to_string(k)]["second"] = (*iah.get_ip_all_hashes_3d_mat().at(i).at(j).at(k)).second;
                    }
                }
            }
            msg["crowd"]["iah"] = contents_j;
            contents_j.clear();

            // Update intro_msg_vec and ip_hemail_vec
            msg["crowd"]["imv"];
            for (auto& el: intro_msg_vec_.get_intro_msg_vec())
            {
                msg["crowd"]["imv"].push_back(*el);
            }

            msg["crowd"]["ihv"];
            for (auto& el: ip_hemail_vec_.get_all_ip_hemail_vec())
            {
                msg["crowd"]["ihv"][(*el).first] = (*el).second;
            }

            ///////
            // Update coin
            ///////
            pl.handle_print_or_log({"Update_you_coin: send all blocks, rocksdb and matrices to server (server)"});

            std::string req_latest_block_coin = buf_j["coin"]["block_nr"];

            // Update blockchain
            ProtocolC protoc;
            msg["coin"]["blocks"] = protoc.get_blocks_from_c(req_latest_block_coin);

            // nlohmann::json list_of_users_j_coin = nlohmann::json::parse(protoc.get_all_users_from_c(req_latest_block_coin));
            // // TODO: there are double nlohmann::json::parse/dumps everywhere
            // // maybe even a stack is better ...

            // // Update rocksdb
            // nlohmann::json rdb;
            // Rocksy* rocksy6 = new Rocksy("transactiondbreadonly");
            // for (auto& user : list_of_users_j_coin)
            // {
            //     nlohmann::json usr;
            //     std::string u = user;
            //     nlohmann::json value_j = nlohmann::json::parse(rocksy6->Get(u));
            //     usr = {u: value_j};
            //     rdb.push_back(usr);
            // }
            // delete rocksy6;

            // msg["coin"]["rocksdb"] = rdb;

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
            // msg["coin"]["bm"] = contents_j;
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
            // msg["coin"]["imm"] = contents_j;
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
            // msg["coin"]["iah"] = contents_j;
            // contents_j.clear();

            // // Update intro_msg_vec and ip_hemail_vec
            // msg["coin"]["imv"];
            // for (auto& el: intro_msg_vec_.get_intro_msg_vec())
            // {
            //     msg["coin"]["imv"].push_back(*el);
            // }

            // msg["coin"]["ihv"];
            // for (auto& el: ip_hemail_vec_.get_all_ip_hemail_vec())
            // {
            //     msg["coin"]["ihv"][(*el).first] = (*el).second;
            // }

            Coin::P2pC pc;
            pc.set_coin_update_complete(true);

            set_resp_msg_server(msg.dump());
        }
        else
        {
            pl.handle_print_or_log({"I'm not the intro online coordinator"});

            // There's another coordinator, send to real coordinator

            nlohmann::json message_j;
            message_j["req"] = "new_co_online";

            Rocksy* rocksy6 = new Rocksy("usersdbreadonly");
            nlohmann::json value_j = nlohmann::json::parse(rocksy6->Get(coordinator_from_hash));
            std::string peer_ip = value_j["ip"];
            delete rocksy6;

            message_j["full_hash_co"] = coordinator_from_hash;
            message_j["ips_shard"] = peer_ip; // TODO adapt became vector
            set_resp_msg_server(message_j.dump());
        }
    }
    else
    {
        pl.handle_print_or_log({"verification intro online user not correct"});
    }

    delete crypto;

    // Disconect from client
    nlohmann::json msg_j;
    msg_j["req"] = "close_this_conn";
    set_resp_msg_server(msg_j.dump());
}

void P2pSession::new_online(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"new peer online:", buf_j["full_hash"], ", inform your bucket"});

    nlohmann::json to_verify_j;
    to_verify_j["req"] = buf_j["req"];
    to_verify_j["full_hash"] = buf_j["full_hash"];
    to_verify_j["ip"] = buf_j["ip"];
    to_verify_j["server"] = buf_j["server"];
    to_verify_j["fullnode"] = buf_j["fullnode"];
    to_verify_j["chosen_one"] = buf_j["chosen_one"];
    to_verify_j["chosen_ones"] = buf_j["chosen_ones"];

    Rocksy* rocksy1 = new Rocksy("usersdbreadonly");
    std::string chosen_one = buf_j["chosen_one"];
    nlohmann::json contents_j = nlohmann::json::parse(rocksy1->Get(chosen_one));
    std::string ecdsa_pub_key_s = contents_j["ecdsa_pub_key"];
    delete rocksy1;

    Crypto* crypto = new Crypto();
    std::string to_verify_s = to_verify_j.dump();
    ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
    crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
    std::string signature = buf_j["signature"];
    std::string signature_bin = crypto->base64_decode(signature);
    
    std::string my_full_hash;
    if (crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
    {
        pl.handle_print_or_log({"verified new online user"});

        FullHash fh;
        my_full_hash = fh.get_full_hash();

        nlohmann::json chosen_ones = buf_j["chosen_ones"];
        bool is_chosen_one  = false;
        for (auto& el: chosen_ones.items())
        {
            if (el.value() == my_full_hash) is_chosen_one = true;
        }

        if (is_chosen_one) // full_hash_coord should be one of the chosen_ones
        {
            pl.handle_print_or_log({"I'm the new online coordinator"});

            // inform the network of new online

            std::string next_full_hash;
            for (int i = 0; i < chosen_ones.size(); i++)
            {
                if (chosen_ones[i] == my_full_hash)
                {
                    if (i != chosen_ones.size() - 1)
                    {
                        next_full_hash = chosen_ones[i+1];
                    }
                    else
                    {
                        next_full_hash = chosen_ones[0];
                    }
                }
            }

            Protocol proto;
            std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, next_full_hash);

            nlohmann::json message_j, to_sign_j;
            message_j["req"] = "new_online";
            message_j["full_hash"] = buf_j["full_hash"];
            message_j["ip"] = buf_j["ip"];
            message_j["server"] = buf_j["server"];
            message_j["fullnode"] = buf_j["fullnode"];
            message_j["chosen_one"] = my_full_hash;

            int k;
            std::string v;
            for (auto &[k, v] : parts)
            {
                message_j["chosen_ones"].push_back(v);
            }

            to_sign_j["req"] = message_j["req"];
            to_sign_j["full_hash"] = message_j["full_hash"];
            to_sign_j["ip"] = message_j["ip"];
            to_sign_j["server"] = message_j["server"];
            to_sign_j["fullnode"] = message_j["fullnode"];
            to_sign_j["chosen_one"] = message_j["chosen_one"];
            to_sign_j["chosen_ones"] = message_j["chosen_ones"];
            std::string to_sign_s = to_sign_j.dump();

            Common::Crypto crypto;
            ECDSA<ECP, SHA256>::PrivateKey private_key;
            std::string signature;
            crypto.ecdsa_load_private_key_from_string(private_key);
            if (crypto.ecdsa_sign_message(private_key, to_sign_s, signature))
            {
                message_j["signature"] = crypto.base64_encode(signature);
            }
            std::string message_s = message_j.dump();

            int key;
            std::string val;
            Poco::BlockMatrix bm;
            P2pNetwork pn;
            Rocksy* rocksy2 = new Rocksy("usersdbreadonly");
            for (int i = 1; i <= parts.size(); i++)
            {
                if (i == 1) continue;

                // lookup in rocksdb
                nlohmann::json value_j = nlohmann::json::parse(rocksy2->Get(parts[i]));
                std::string peer_ip = value_j["ip"];

                // inform the underlying network
                if (parts[i] == my_full_hash) // TODO the else part isn't activated, dunno why, search in test terminals for new_peer
                {
                    // inform server's underlying network
                    pl.handle_print_or_log({"Send new_online req: Inform my underlying network as coordinator"});

                    std::string next_hash;
                    if (i != parts.size())
                    {
                        next_hash = parts[i+1];
                    }
                    else
                    {
                        next_hash = parts[1];
                    }
                    
                    std::map<int, std::string> parts_underlying = proto.partition_in_buckets(my_full_hash, next_hash);
                    std::string key2, val2;
                    for (int i = 1; i <= parts_underlying.size(); i++)
                    {
                        if (i == 1) continue; // ugly hack for a problem in proto.partition_in_buckets()
                        if (parts_underlying[i] == my_full_hash) continue;

                        // lookup in rocksdb
                        std::string val2 = parts_underlying[i];
                        nlohmann::json value_j = nlohmann::json::parse(rocksy2->Get(val2));
                        std::string ip_underlying = value_j["ip"];

                        pl.handle_print_or_log({"Send new_online req: Non-connected underlying peers - client: ", ip_underlying});

                        Poco::PocoCrowd pc;
                        bool cont = false;
                        for (auto& el: pc.get_new_users_ip())
                        {
                            if (el == ip_underlying)
                            {
                                cont = true;
                                break;
                            }
                        }
                        if (cont) continue;

                        // message to non-connected peers
                        std::string message = message_j.dump();
                        pn.p2p_client(ip_underlying, message);
                    }
                }
                
                pl.handle_print_or_log({"Preparation for new_online:", peer_ip});

                Poco::PocoCrowd pc;
                bool cont = false;
                for (auto& el: pc.get_new_users_ip())
                {
                    if (el == peer_ip)
                    {
                        cont = true;
                        break;
                    }
                }
                if (cont) continue;

                // p2p_client() to all chosen ones
                pn.p2p_client(peer_ip, message_s);
            }

            delete rocksy2;

            // update this rocksdb
            Rocksy* rocksy3 = new Rocksy("usersdb");
            std::string full_hash = buf_j["full_hash"];
            nlohmann::json value_j = nlohmann::json::parse(rocksy3->Get(full_hash));
            value_j["online"] = "true";
            std::string value_s = value_j.dump();
            rocksy3->Put(full_hash, value_s);
            delete rocksy3;
        }
        else
        {
            pl.handle_print_or_log({"I'm not the new online coordinator"});
        }
    }
    else
    {
        pl.handle_print_or_log({"verification new online user not correct"});
    }

    delete crypto;

    // Disconect from client
    nlohmann::json msg_j;
    msg_j["req"] = "close_this_conn";
    set_resp_msg_server(msg_j.dump());
}

void P2pSession::intro_offline(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"intro peer offline: ", buf_j["full_hash"]});
    
    nlohmann::json to_verify_j;
    to_verify_j["req"] = buf_j["req"];
    to_verify_j["full_hash"] = buf_j["full_hash"];

    Rocksy* rocksy1 = new Rocksy("usersdbreadonly"); // TODO maybe isolate all rocksy calls in intro_offline, new_offline, intro_online and new_online as they might block
    std::string full_hash = buf_j["full_hash"];
    nlohmann::json contents_j = nlohmann::json::parse(rocksy1->Get(full_hash));
    std::string ecdsa_pub_key_s = contents_j["ecdsa_pub_key"];
    delete rocksy1;

    Crypto* crypto = new Crypto();
    std::string to_verify_s = to_verify_j.dump();
    ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
    crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
    std::string signature = buf_j["signature"];
    std::string signature_bin = crypto->base64_decode(signature);
    
    std::string my_full_hash;
    if (crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
    {
        pl.handle_print_or_log({"verified intro offline user"});

        PrevHash ph;
        std::string next_prev_hash = ph.calculate_hash_from_last_block();

        std::string msg_and_nph = buf_j.dump() + next_prev_hash;
        std::string hash_msg_and_nph =  crypto->bech32_encode_sha256(msg_and_nph);

        FullHash fh;
        my_full_hash = fh.get_full_hash();

        Rocksy* rocksy2 = new Rocksy("usersdbreadonly");
        std::string coordinator_from_hash = rocksy2->FindCoordinator(hash_msg_and_nph);
        delete rocksy2;

        pl.handle_print_or_log({"my_full_hash: ", my_full_hash});
        pl.handle_print_or_log({"coordinator_from_hash: ", coordinator_from_hash});

        if (my_full_hash == coordinator_from_hash)
        {
            pl.handle_print_or_log({"I'm the intro offline coordinator"});

            // inform the network of new offline
            Protocol proto;

            std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

            nlohmann::json message_j, to_sign_j;
            message_j["req"] = "new_offline";
            message_j["full_hash"] = full_hash;
            message_j["chosen_one"] = my_full_hash;

            int k;
            std::string v;
            for (auto &[k, v] : parts)
            {
                message_j["chosen_ones"].push_back(v);
            }

            to_sign_j["req"] = message_j["req"];
            to_sign_j["full_hash"] = full_hash;
            to_sign_j["chosen_one"] = my_full_hash;
            to_sign_j["chosen_ones"] = message_j["chosen_ones"];
            std::string to_sign_s = to_sign_j.dump();

            Common::Crypto crypto;
            ECDSA<ECP, SHA256>::PrivateKey private_key;
            std::string signature;
            crypto.ecdsa_load_private_key_from_string(private_key);
            if (crypto.ecdsa_sign_message(private_key, to_sign_s, signature))
            {
                message_j["signature"] = crypto.base64_encode(signature);
            }
            std::string message_s = message_j.dump();

            int key;
            std::string val;
            Poco::BlockMatrix bm;
            P2pNetwork pn;
            Rocksy* rocksy3 = new Rocksy("usersdbreadonly");
            for (int i = 1; i <= parts.size(); i++)
            {
                if (i == 1) continue;

                // lookup in rocksdb
                nlohmann::json value_j = nlohmann::json::parse(rocksy3->Get(parts[i]));
                std::string peer_ip = value_j["ip"];

                // inform the underlying network
                if (parts[i] == my_full_hash) // TODO the else part isn't activated, dunno why, search in test terminals for new_peer
                {
                    // inform server's underlying network
                    pl.handle_print_or_log({"Send intro_offline req: Inform my underlying network as coordinator"});

                    std::string next_hash;
                    if (i != parts.size())
                    {
                        next_hash = parts[i+1];
                    }
                    else
                    {
                        next_hash = parts[1];
                    }
                    
                    std::map<int, std::string> parts_underlying = proto.partition_in_buckets(my_full_hash, next_hash);
                    std::string key2, val2;
                    for (int i = 1; i <= parts_underlying.size(); i++)
                    {
                        if (i == 1) continue; // ugly hack for a problem in proto.partition_in_buckets()
                        if (parts_underlying[i] == my_full_hash) continue;

                        // lookup in rocksdb
                        std::string val2 = parts_underlying[i];
                        nlohmann::json value_j = nlohmann::json::parse(rocksy3->Get(val2));
                        std::string ip_underlying = value_j["ip"];

                        pl.handle_print_or_log({"Send intro_offline req: Non-connected underlying peers - client: ", ip_underlying});

                        Poco::PocoCrowd pc;
                        bool cont = false;
                        for (auto& el: pc.get_new_users_ip())
                        {
                            if (el == ip_underlying)
                            {
                                cont = true;
                                break;
                            }
                        }
                        if (cont) continue;

                        // message to non-connected peers
                        std::string message = message_j.dump();
                        pn.p2p_client(ip_underlying, message);
                    }
                }
                
                pl.handle_print_or_log({"Preparation for new_offline:", peer_ip});

                Poco::PocoCrowd pc;
                bool cont = false;
                for (auto& el: pc.get_new_users_ip())
                {
                    if (el == peer_ip)
                    {
                        cont = true;
                        break;
                    }
                }
                if (cont) continue;

                // p2p_client() to all chosen ones
                pn.p2p_client(peer_ip, message_s);
            }

            delete rocksy3;

            // update this rocksdb
            Rocksy* rocksy4 = new Rocksy("usersdb");
            nlohmann::json value_j = nlohmann::json::parse(rocksy4->Get(full_hash));
            value_j["online"] = "false";
            std::string value_s = value_j.dump();
            rocksy4->Put(full_hash, value_s);
            delete rocksy4;
        }
        else
        {
            pl.handle_print_or_log({"I'm not the intro offline coordinator"});

            // There's another coordinator, send to real coordinator

            nlohmann::json message_j;
            message_j["req"] = "new_co_offline";

            Rocksy* rocksy5 = new Rocksy("usersdbreadonly");
            nlohmann::json value_j = nlohmann::json::parse(rocksy5->Get(coordinator_from_hash));
            std::string peer_ip = value_j["ip"];
            delete rocksy5;

            message_j["full_hash_co"] = coordinator_from_hash;
            message_j["ips_shard"] = peer_ip; // TODO adapt became vector
            set_resp_msg_server(message_j.dump());
        }
    }
    else
    {
        pl.handle_print_or_log({"verification intro offline user not correct"});
    }

    delete crypto;

    // Disconect from client
    nlohmann::json msg_j;
    msg_j["req"] = "close_this_conn";
    set_resp_msg_server(msg_j.dump());

    // Ctrl-c for when the requestor receives an intro_offline message --> then the circle is round
    // Graciously terminating the program
    if (my_full_hash == full_hash)
    {
        pl.handle_print_or_log({"exit intro offline has run"});
        exit(2);
    }
}

void P2pSession::new_offline(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"new peer offline:", buf_j["full_hash"], ", inform your bucket"});

    nlohmann::json to_verify_j;
    to_verify_j["req"] = buf_j["req"];
    to_verify_j["full_hash"] = buf_j["full_hash"];
    to_verify_j["chosen_one"] = buf_j["chosen_one"];
    to_verify_j["chosen_ones"] = buf_j["chosen_ones"];

    Rocksy* rocksy1 = new Rocksy("usersdbreadonly");
    std::string chosen_one = buf_j["chosen_one"];
    nlohmann::json contents_j = nlohmann::json::parse(rocksy1->Get(chosen_one));
    std::string ecdsa_pub_key_s = contents_j["ecdsa_pub_key"];
    delete rocksy1;

    Crypto* crypto = new Crypto();
    std::string to_verify_s = to_verify_j.dump();
    ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
    crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
    std::string signature = buf_j["signature"];
    std::string signature_bin = crypto->base64_decode(signature);
    
    std::string my_full_hash;
    if (crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
    {
        pl.handle_print_or_log({"verified new offline user"});

        FullHash fh;
        my_full_hash = fh.get_full_hash();

        nlohmann::json chosen_ones = buf_j["chosen_ones"];
        bool is_chosen_one  = false;
        for (auto& el: chosen_ones.items())
        {
            if (el.value() == my_full_hash) is_chosen_one = true;
        }

        if (is_chosen_one) // full_hash_coord should be one of the chosen_ones
        {
            pl.handle_print_or_log({"I'm the new offline coordinator"});

            // inform the network of new offline

            std::string next_full_hash;
            for (int i = 0; i < chosen_ones.size(); i++)
            {
                if (chosen_ones[i] == my_full_hash)
                {
                    if (i != chosen_ones.size() - 1)
                    {
                        next_full_hash = chosen_ones[i+1];
                    }
                    else
                    {
                        next_full_hash = chosen_ones[0];
                    }
                }
            }

            Protocol proto;
            std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, next_full_hash);

            nlohmann::json message_j, to_sign_j;
            message_j["req"] = "new_offline";
            message_j["full_hash"] = buf_j["full_hash"];
            message_j["chosen_one"] = my_full_hash;

            int k;
            std::string v;
            for (auto &[k, v] : parts)
            {
                message_j["chosen_ones"].push_back(v);
            }

            to_sign_j["req"] = message_j["req"];
            to_sign_j["full_hash"] = message_j["full_hash"];
            to_sign_j["chosen_one"] = message_j["chosen_one"];
            to_sign_j["chosen_ones"] = message_j["chosen_ones"];
            std::string to_sign_s = to_sign_j.dump();

            Common::Crypto crypto;
            ECDSA<ECP, SHA256>::PrivateKey private_key;
            std::string signature;
            crypto.ecdsa_load_private_key_from_string(private_key);
            if (crypto.ecdsa_sign_message(private_key, to_sign_s, signature))
            {
                message_j["signature"] = crypto.base64_encode(signature);
            }
            std::string message_s = message_j.dump();

            int key;
            std::string val;
            Poco::BlockMatrix bm;
            P2pNetwork pn;
            Rocksy* rocksy2 = new Rocksy("usersdbreadonly");
            for (int i = 1; i <= parts.size(); i++)
            {
                if (i == 1) continue;

                // lookup in rocksdb
                nlohmann::json value_j = nlohmann::json::parse(rocksy2->Get(parts[i]));
                std::string peer_ip = value_j["ip"];

                // inform the underlying network
                if (parts[i] == my_full_hash) // TODO the else part isn't activated, dunno why, search in test terminals for new_peer
                {
                    // inform server's underlying network
                    pl.handle_print_or_log({"Send new_offline req: Inform my underlying network as coordinator"});

                    std::string next_hash;
                    if (i != parts.size())
                    {
                        next_hash = parts[i+1];
                    }
                    else
                    {
                        next_hash = parts[1];
                    }
                    
                    std::map<int, std::string> parts_underlying = proto.partition_in_buckets(my_full_hash, next_hash);
                    std::string key2, val2;
                    for (int i = 1; i <= parts_underlying.size(); i++)
                    {
                        if (i == 1) continue; // ugly hack for a problem in proto.partition_in_buckets()
                        if (parts_underlying[i] == my_full_hash) continue;

                        // lookup in rocksdb
                        std::string val2 = parts_underlying[i];
                        nlohmann::json value_j = nlohmann::json::parse(rocksy2->Get(val2));
                        std::string ip_underlying = value_j["ip"];

                        pl.handle_print_or_log({"Send new_offline req: Non-connected underlying peers - client: ", ip_underlying});

                        Poco::PocoCrowd pc;
                        bool cont = false;
                        for (auto& el: pc.get_new_users_ip())
                        {
                            if (el == ip_underlying)
                            {
                                cont = true;
                                break;
                            }
                        }
                        if (cont) continue;

                        // message to non-connected peers
                        std::string message = message_j.dump();
                        pn.p2p_client(ip_underlying, message);
                    }
                }
                
                pl.handle_print_or_log({"Preparation for new_offline:", peer_ip});

                Poco::PocoCrowd pc;
                bool cont = false;
                for (auto& el: pc.get_new_users_ip())
                {
                    if (el == peer_ip)
                    {
                        cont = true;
                        break;
                    }
                }
                if (cont) continue;

                // p2p_client() to all chosen ones
                pn.p2p_client(peer_ip, message_s);
            }

            delete rocksy2;

            // update this rocksdb
            Rocksy* rocksy3 = new Rocksy("usersdb");
            std::string full_hash = buf_j["full_hash"];
            nlohmann::json value_j = nlohmann::json::parse(rocksy3->Get(full_hash));
            value_j["online"] = "false";
            std::string value_s = value_j.dump();
            rocksy3->Put(full_hash, value_s);
            delete rocksy3;
        }
        else
        {
            pl.handle_print_or_log({"I'm not the new offline coordinator"});
        }
    }
    else
    {
        pl.handle_print_or_log({"verification new offline user not correct"});
    }

    delete crypto;

    // Disconect from client
    nlohmann::json msg_j;
    msg_j["req"] = "close_this_conn";
    set_resp_msg_server(msg_j.dump());

    std::string full_hash = buf_j["full_hash"];

    // Ctrl-c for when the requestor receives an new_offline message --> then the circle is round
    // Graciously terminating the program
    if (my_full_hash == full_hash)
    {
        pl.handle_print_or_log({"exit new offline has run"});
        exit(2);
    }
}

void P2pSession::update_you_server(nlohmann::json buf_j)
{
    Common::Print_or_log pl;

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());

    ///////////
    // Update crowd
    ///////////
    pl.handle_print_or_log({"Update_me_crowd: receive all blocks, rocksdb and matrices from server (server)"});

    // Update blocks
    nlohmann::json blocks_j = buf_j["crowd"]["blocks"];
    for (auto& b: blocks_j.items())
    {
        nlohmann::json block_j = b.value()["block"];
        std::string block_nr = b.value()["block_nr"];

        merkle_tree mt;
        mt.save_block_to_file(block_j, block_nr);
    }

    // Update rocksdb
    nlohmann::json rdb_j = buf_j["crowd"]["rocksdb"];

    for (auto& element : rdb_j)
    {
        std::string key_s = element["full_hash"];
        std::string value_s = element.dump();

        Rocksy* rocksy = new Rocksy("usersdb");
        rocksy->Put(key_s, value_s);
        delete rocksy;
    }

    // Update matrices
    nlohmann::json block_matrix_j = buf_j["crowd"]["bm"];
    nlohmann::json intro_msg_s_matrix_j = buf_j["crowd"]["imm"];
    nlohmann::json ip_all_hashes_j = buf_j["crowd"]["iah"];

    Poco::BlockMatrix bm;
    Poco::IntroMsgsMat imm;
    Poco::IpAllHashes iah;

    bm.get_block_matrix().clear(); // TODO clear the matrix --> this doesn't clear it
    bm.get_calculated_hash_matrix().clear();
    bm.get_prev_hash_matrix().clear();

    for (auto& [k1, v1] : block_matrix_j.items())
    {
        for (auto& [k2, v2] : v1.items())
        {
            bm.add_block_to_block_vector(v2);
            bm.add_calculated_hash_to_calculated_hash_vector(v2);
            bm.add_prev_hash_to_prev_hash_vector(v2);
        }

        bm.add_block_vector_to_block_matrix();
        bm.add_calculated_hash_vector_to_calculated_hash_matrix();
        bm.add_prev_hash_vector_to_prev_hash_matrix();
    }

    for (auto& [k1, v1] : intro_msg_s_matrix_j.items())
    {
        for (auto& [k2, v2] : v1.items())
        {
            for (auto& [k3, v3] : v2.items())
            {
                imm.add_intro_msg_to_intro_msg_s_vec(v3);
            }

            imm.add_intro_msg_s_vec_to_intro_msg_s_2d_mat();
        }

        imm.add_intro_msg_s_2d_mat_to_intro_msg_s_3d_mat();
    }

    for (auto& [k1, v1] : ip_all_hashes_j.items())
    {
        for (auto& [k2, v2] : v1.items())
        {
            for (auto& [k3, v3] : v2.items())
            {
                std::pair<std::string, std::string> myPair = std::make_pair(v3["first"], v3["second"]);
                std::shared_ptr<std::pair<std::string, std::string>> ptr(new std::pair<std::string, std::string> (myPair));
                iah.add_ip_hemail_to_ip_all_hashes_vec(ptr);
            }

            iah.add_ip_all_hashes_vec_to_ip_all_hashes_2d_mat();
        }

        iah.add_ip_all_hashes_2d_mat_to_ip_all_hashes_3d_mat();
    }

    // Update intro_msg_vec and ip_hemail_vec
    nlohmann::json intro_msg_vec_j = buf_j["crowd"]["imv"];
    nlohmann::json ip_hemail_vec_j = buf_j["crowd"]["ihv"];

    for (auto& el: intro_msg_vec_j)
    {
        intro_msg_vec_.add_to_intro_msg_vec(el);
    }

    for (auto& [k, v]: ip_hemail_vec_j.items())
    {
        ip_hemail_vec_.add_ip_hemail_to_ip_hemail_vec(k, v);
    }

    ///////////
    // Update coin
    ///////////
    pl.handle_print_or_log({"Update_me_coin: receive all blocks, rocksdb and matrices from server (server)"});

    // // Update blocks
    // nlohmann::json blocks_j = buf_j["coin"]["blocks"];
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

    P2pC pc;
    pc.set_coin_update_complete(true);
}

void P2pSession::hello_tx(nlohmann::json buf_j)
{
    //
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Hello_tx:"});

    std::string full_hash_req = buf_j["full_hash_req"];
    std::string to_full_hash = buf_j["tx_to"];
    std::string amount = buf_j["amount"];
    std::string signature = buf_j["signature"];

    Rocksy* rocksy = new Rocksy("usersdbreadonly");
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(full_hash_req));
    if (contents_j == "")
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Requester not in database"});
        
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
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Hello_tx: verified"});
        
        FullHash fh;
        std::string my_full_hash = fh.get_full_hash();

        PrevHashC phc;
        std::string hash_latest_block = phc.calculate_hash_from_last_block_c();
        std::string prel_coordinator = full_hash_req + hash_latest_block;

        Rocksy* rocksy = new Rocksy("usersdbreadonly");
        std::string full_hash_coordinator = rocksy->FindCoordinator(prel_coordinator);
        delete rocksy;
        
        if (my_full_hash == full_hash_coordinator && my_full_hash != full_hash_req)
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Hello_tx: I'm the coordinator"});
            
            // Are funds sufficient? Create a second rocksdb here!
            Rocksy* rocksy = new Rocksy("transactionsdbreadonly");
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
                Common::Print_or_log pl;
                pl.handle_print_or_log({"funds are ok"});
                
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
                    if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
                    
                    Rocksy* rocksy = new Rocksy("usersdbreadonly");

                    // lookup in rocksdb
                    nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
                    std::string peer_ip = value_j["ip"];

                    delete rocksy;
                    
                    std::string message = message_j.dump();

                    Common::Print_or_log pl;
                    pl.handle_print_or_log({"Preparation for intro_tx:", peer_ip});
                    
                    // p2p_client() to all chosen ones with intro_tx request
                    pn.p2p_client(peer_ip, message);
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
                Common::Print_or_log pl;
                pl.handle_print_or_log({"Hello_tx: funds don't suffice"});
            }
        }
        else
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Hello_tx: I'm not the coordinator, try again in a few minutes"});

            //TODO to new_co
        }
    }
    else
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Hello_tx: verification didn't succeed"});
    }

    delete crypto;
}

void P2pSession::intro_tx(nlohmann::json buf_j)
{
    //
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Intro_tx:"});
    
    std::string full_hash_req = buf_j["full_hash_req"];
    std::string to_full_hash = buf_j["tx_to"];
    std::string amount = buf_j["amount"];
    std::string signature = buf_j["signature"];
    nlohmann::json chosen_ones = buf_j["chosen_ones"];

    // introduce chosen ones!!! and coordinator should be verified

    Rocksy* rocksy = new Rocksy("usersdbreadonly");
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(full_hash_req));
    if (contents_j == "")
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Requester not in database"});
        
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
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Intro_tx: verified"});
        
        FullHash fh;
        std::string my_full_hash = fh.get_full_hash();

        PrevHashC phc;
        std::string hash_latest_block = phc.calculate_hash_from_last_block_c();
        std::string coordinator = full_hash_req + hash_latest_block;

        bool is_chosen_one = false;
        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (chosen_ones[i] == my_full_hash) is_chosen_one = true;
        }

        if (is_chosen_one && my_full_hash != full_hash_req)
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Intro_tx: I'm a chosen_one"});
            
            // Are funds sufficient?
            Rocksy* rocksy = new Rocksy("transactionsdbreadonly");
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
                Common::Print_or_log pl;
                pl.handle_print_or_log({"funds are ok"});
                
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
                    if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
                    
                    Rocksy* rocksy = new Rocksy("usersdbreadonly");

                    // lookup in rocksdb
                    nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
                    std::string peer_ip = value_j["ip"];

                    delete rocksy;
                    
                    std::string message = message_j.dump();

                    Common::Print_or_log pl;
                    pl.handle_print_or_log({"Preparation for new_tx:", peer_ip});
                    
                    // p2p_client() to all chosen ones with intro_tx request
                    pn.p2p_client(peer_ip, message);
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
                Common::Print_or_log pl;
                pl.handle_print_or_log({"Intro_tx: funds don't suffice"});
            }
        }
        else
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Intro_tx: I'm not chosen_one, maybe try again in a few minutes"});
        }
    }
    else
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Intro_tx: verification didn't succeed"});
    }

    delete crypto;
}

void P2pSession::new_tx(nlohmann::json buf_j)
{
    //
    Common::Print_or_log pl;
    pl.handle_print_or_log({"New_tx:"});
    
    std::string full_hash_req = buf_j["full_hash_req"];
    std::string to_full_hash = buf_j["tx_to"];
    std::string amount = buf_j["amount"];
    std::string signature = buf_j["signature"];
    nlohmann::json chosen_ones = buf_j["chosen_ones"];

    // introduce chosen ones!!! and coordinator should be verified

    Rocksy* rocksy = new Rocksy("usersdbreadonly");
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(full_hash_req));
    if (contents_j == "")
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Requester not in database"});
        
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
        Common::Print_or_log pl;
        pl.handle_print_or_log({"New_tx: verified"});
        
        FullHash fh;
        std::string my_full_hash = fh.get_full_hash();

        PrevHashC phc;
        std::string hash_latest_block = phc.calculate_hash_from_last_block_c();
        std::string coordinator = full_hash_req + hash_latest_block;

        bool is_chosen_one = false;
        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (chosen_ones[i] == my_full_hash) is_chosen_one = true;
        }

        if (is_chosen_one && my_full_hash != full_hash_req)
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"New_tx: I'm a secondary chosen_one"});
            
            // Are funds sufficient?
            Rocksy* rocksy = new Rocksy("transactionsdbreadonly");
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
                Common::Print_or_log pl;
                pl.handle_print_or_log({"funds are ok"});
                
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
                    if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
                    
                    Rocksy* rocksy = new Rocksy("usersdbreadonly");

                    // lookup in rocksdb
                    nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
                    std::string peer_ip = value_j["ip"];

                    delete rocksy;
                    
                    std::string message = message_j.dump();

                    Common::Print_or_log pl;
                    pl.handle_print_or_log({"Preparation for new_tx:", peer_ip});
                    
                    // p2p_client() to all chosen ones with new_tx request
                    pn.p2p_client(peer_ip, message);
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
                Common::Print_or_log pl;
                pl.handle_print_or_log({"New_tx: funds don't suffice"});
            }
        }
        else
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"New_tx: I'm not a secondary chosen_one, maybe try again in a few minutes"});
        }
    }
    else
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"New_tx: verification didn't succeed"});
    }

    delete crypto;
}

void P2pSession::hello_reward(nlohmann::json buf_j)
{
    //
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Hello_reward:"});
    
    std::string full_hash_req = buf_j["full_hash_req"];
    std::string hash_of_block = buf_j["hash_of_block"];
    nlohmann::json chosen_ones_reward = buf_j["chosen_ones_reward"];
    std::string signature = buf_j["signature"];

    Rocksy* rocksy = new Rocksy("usersdbreadonly");
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(full_hash_req));
    if (contents_j == "")
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Requester not in database"});
        
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
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Hello_reward: verified"});
        
        FullHash fh;
        std::string my_full_hash = fh.get_full_hash();

        Rocksy* rocksy = new Rocksy("usersdbreadonly");
        std::string chosen_ones_s = chosen_ones_reward.dump();
        std::string hash_of_cos = crypto->bech32_encode_sha256(chosen_ones_s);
        std::string coordinator = rocksy->FindCoordinator(hash_of_cos);
        delete rocksy;
        
        if (my_full_hash == coordinator)
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Hello_reward: I'm the coordinator"});
            
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
                if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
                
                Rocksy* rocksy = new Rocksy("usersdbreadonly");

                // lookup in rocksdb
                nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
                std::string peer_ip = value_j["ip"];

                delete rocksy;
                
                std::string message = message_j.dump();

                Common::Print_or_log pl;
                pl.handle_print_or_log({"Preparation for intro_reward:", peer_ip});
                
                // p2p_client() to all chosen ones with intro_reward request
                pn.p2p_client(peer_ip, message);
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
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Hello_reward: I'm not the coordinator, try again in a few minutes"});
        }
    }
    else
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Hello_reward: verification didn't succeed"});
    }

    delete crypto;
}

void P2pSession::intro_reward(nlohmann::json buf_j)
{
    //
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Intro_reward:"});
    
    std::string hash_of_block = buf_j["hash_of_block"];
    nlohmann::json chosen_ones_reward = buf_j["chosen_ones_reward"];
    std::string signature = buf_j["signature"];
    nlohmann::json chosen_ones = buf_j["chosen_ones"];

    // introduce chosen ones!!! and coordinator should be verified

    Rocksy* rocksy = new Rocksy("usersdbreadonly");
    std::string coordinator = chosen_ones[0]; // TODO should be element of chosen_ones as the json is sorted
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(coordinator));
    if (contents_j == "")
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Requester not in database"});
        
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
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Intro_reward: verified"});
        
        FullHash fh;
        std::string my_full_hash = fh.get_full_hash();

        bool is_chosen_one = false;
        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (chosen_ones[i] == my_full_hash) is_chosen_one = true;
        }
        
        if (is_chosen_one)
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Intro_reward: I'm a chosen_one"});

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
                if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
                
                Rocksy* rocksy = new Rocksy("usersdbreadonly");

                // lookup in rocksdb
                nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
                std::string peer_ip = value_j["ip"];

                delete rocksy;
                
                std::string message = message_j.dump();

                Common::Print_or_log pl;
                pl.handle_print_or_log({"Preparation for new_reward:", peer_ip});
                
                // p2p_client() to all chosen ones with intro_reward request
                pn.p2p_client(peer_ip, message);
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
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Intro_reward: I'm not chosen_one, maybe try again in a few minutes"});
        }
    }
    else
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Intro_reward: verification didn't succeed"});
    }

    delete crypto;
}

void P2pSession::new_reward(nlohmann::json buf_j)
{
    //
    Common::Print_or_log pl;
    pl.handle_print_or_log({"New_reward:"});

    std::string hash_of_block = buf_j["hash_of_block"];
    nlohmann::json chosen_ones_reward = buf_j["chosen_ones_reward"];
    std::string signature = buf_j["signature"];
    nlohmann::json chosen_ones = buf_j["chosen_ones"];

    // introduce chosen ones!!! and coordinator should be verified

    Rocksy* rocksy = new Rocksy("usersdbreadonly");
    std::string coordinator = chosen_ones[0]; // TODO should be element of chosen_ones as the json is sorted
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(coordinator));
    if (contents_j == "")
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Requester not in database"});
        
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
        Common::Print_or_log pl;
        pl.handle_print_or_log({"New_reward: verified"});
        
        FullHash fh;
        std::string my_full_hash = fh.get_full_hash();

        bool is_chosen_one = false;
        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (chosen_ones[i] == my_full_hash) is_chosen_one = true;
        }
        
        if (is_chosen_one)
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Intro_reward: I'm a secondary chosen_one"});
            
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
                if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
                
                Rocksy* rocksy = new Rocksy("usersdbreadonly");

                // lookup in rocksdb
                nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
                std::string peer_ip = value_j["ip"];

                delete rocksy;
                
                std::string message = message_j.dump();

                Common::Print_or_log pl;
                pl.handle_print_or_log({"Preparation for new_reward:", peer_ip});

                // p2p_client() to all chosen ones with intro_reward request
                pn.p2p_client(peer_ip, message);
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
            Common::Print_or_log pl;
            pl.handle_print_or_log({"New_reward: I'm not a secondary chosen_one, maybe try again in a few minutes"});
        }
    }
    else
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"New_reward: verification didn't succeed"});
    }

    delete crypto;
}

void P2pSession::start_block_creation_thread()
{
    Transactions *tx = new Transactions();

    if (tx->get_transactions().size() > 2048) // size limit of block within block creation delay
    {
        // Create block
        PocoCoin poco;
        poco.create_prel_blocks_c();

        tx->reset_transactions();
    }
    else if (tx->get_transactions().size() == 1 || tx->get_new_rewards())
    {
        // wait 20 secs
        // then create block

        Common::Print_or_log pl;
        pl.handle_print_or_log({"Get_sleep_and_create_block_c"});
        
        std::thread t(&P2pSession::get_sleep_and_create_block_server_c, this);
        t.detach();
    }

    delete tx;
}

void P2pSession::get_sleep_and_create_block_server_c()
{
    std::this_thread::sleep_for(std::chrono::seconds(10));

    Transactions tx;
    Common::Print_or_log pl;
    pl.handle_print_or_log({"transactions.size() in Coin:", to_string(tx.get_transactions().size())});
    
    BlockMatrixC *bmc = new BlockMatrixC();
    bmc->add_received_block_vector_to_received_block_matrix();

    PocoCoin poco;
    poco.create_prel_blocks_c(); // chosen ones are being informed here

    pl.handle_print_or_log({"Block_c created server!!"});
    
    delete bmc;
}

void P2pSession::intro_block_c(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Intro_block_c:"});
    
    // Compare the hashes from the block of the coordinator with this block
    // p2p_client to other chosen_ones --> needs to be calculated depending on this server's place in the chosen_ones list
    // Communicate hash to all
    // Then inform your underlying network
    // Put block in waiting list until it's the only block in the chain --> future implementation with coin

    // Is the coordinator the truthful real coordinator for this block

    nlohmann::json recv_block_j = buf_j["block"];
    BlockMatrixC *bmc = new BlockMatrixC();
    if (bmc->get_block_matrix().empty())
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Received block is in block_vector; 1"});
        
        bmc->add_received_block_to_received_block_vector(recv_block_j);
    }
    else
    {
        for (uint16_t i = 0; i < bmc->get_block_matrix().back().size(); i++)
        {
            if (*bmc->get_block_matrix().back().at(i) == recv_block_j)
            {
                Common::Print_or_log pl;
                pl.handle_print_or_log({"Received block is in block_vector; 2"});
                
                bmc->add_received_block_to_received_block_vector(recv_block_j);
                break;
            }
            
            if (i == bmc->get_block_matrix().back().size() - 1)
            {
                // Don't accept this block
                Common::Print_or_log pl;
                pl.handle_print_or_log({"Received block not in block_vector"});
                                
                return;
            }
        }
    }

    delete bmc;

    std::string full_hash_coord_from_coord = buf_j["full_hash_coord"];

    nlohmann::json starttime_coord = buf_j["block"]["starttime"];

    Common::Crypto crypto;

    recv_block_j["starttime"] = starttime_coord;
    std::string recv_block_s = recv_block_j.dump();
    std::string prev_hash_me = crypto.bech32_encode_sha256(recv_block_s);

    Rocksy* rocksy = new Rocksy("usersdbreadonly");
    std::string full_hash_coord_from_me = rocksy->FindCoordinator(prev_hash_me);
    if (full_hash_coord_from_me == buf_j["full_hash_req"])
    {
        full_hash_coord_from_me = rocksy->FindNextPeer(full_hash_coord_from_me);
    }
    delete rocksy;

    FullHash fh;
    std::string my_full_hash = fh.get_full_hash();

    if (full_hash_coord_from_coord == full_hash_coord_from_me)
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Coordinator is truthful c"});
        
        std::string prev_hash_coordinator = buf_j["prev_hash"];
        std::string prev_hash_in_block = buf_j["block"]["prev_hash"];

        if (prev_hash_coordinator == prev_hash_in_block && prev_hash_coordinator == prev_hash_me)
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Successful comparison of prev_hashes, now sharing hashes c"});
            
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
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Unsuccessful comparison of prev_hashes c"});
        }

        // Inform coordinator of succesfullness of hash comparison
        nlohmann::json m_j;
        m_j["req"] = "hash_comparison_c";
        m_j["hash_comp"] = prev_hash_in_block == prev_hash_coordinator;
        std::string msg_s = m_j.dump();
//TODO
//P2pNetwork::set_resp_msg_server(msg_s);
//
        // p2p_client() to all calculated other chosen_ones
        // this is in fact the start of the consensus algorithm
        // you don't need full consensus in order to create a succesful block
        // but full consensus improves your chances of course greatly
        nlohmann::json chosen_ones = buf_j["chosen_ones"];

        FullHash fh;
        std::string my_full_hash = fh.get_full_hash(); // TODO this is a file lookup and thus takes time --> static var should be
        // pl.handle_print_or_log({"My_full_hash already present in file:", my_full_hash});

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
                Rocksy* rocksy = new Rocksy("usersdbreadonly");
                nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(c_one));
                delete rocksy;

                std::string ip = value_j["ip"];
                
                nlohmann::json msg_j;
                msg_j["req"] = "hash_comparison_c";
                msg_j["hash_comp"] = prev_hash_me == prev_hash_coordinator;
                std::string msg_s = msg_j.dump();

                P2pNetwork pn;
                pn.p2p_client(ip, msg_s);
            }
            else if (i == j)
            {
                continue;
            }
        }
    }
    else
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"Coordinator is not truthful c"});
    }

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
//TODO
//set_resp_msg_server(m_j.dump());
//
}

void P2pSession::hash_comparison_c(nlohmann::json buf_j)
{
    // compare the received hash
    Common::Print_or_log pl;
    pl.handle_print_or_log({"The hash comparison is (server):", buf_j["hash_comp"]});
}

void P2pSession::new_block_c(nlohmann::json buf_j)
{
    // new_block --> TODO block and rocksdb should be saved
    Common::Print_or_log pl;
    pl.handle_print_or_log({"New_block_c:"});
}

void P2pSession::set_resp_msg_server(std::string msg)
{
    P2pNetwork pn;
    std::vector<std::string> splitted = pn.split(msg, p2p_message::max_body_length);
    p2p_message resp_msg_server;
    for (int i = 0; i < splitted.size(); i++)
    {
        char s[p2p_message::max_body_length];
        strncpy(s, splitted[i].c_str(), sizeof(s));

        resp_msg_server.body_length(std::strlen(s));
        std::memcpy(resp_msg_server.body(), s, resp_msg_server.body_length());
        i == splitted.size() - 1 ? resp_msg_server.encode_header(1) : resp_msg_server.encode_header(0); // 1 indicates end of message eom, TODO perhaps a set_eom_flag(true) instead of an int

        deliver(resp_msg_server);
    }
}

//----------------------------------------------------------------------

P2pSession::P2pSession(tcp::socket socket) : socket_(std::move(socket))
{
}

void P2pSession::start()
{
    do_read_header();
}

void P2pSession::deliver(const p2p_message &msg)
{
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
        do_write();
    }
}

void P2pSession::do_read_header()
{
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_msg_.data(), p2p_message::header_length),
                            [this, self](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec && read_msg_.decode_header())
                                {
                                    do_read_body();
                                }
                                else
                                {
                                }
                            });
}

void P2pSession::do_read_body()
{
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                            [this, self](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec)
                                {
                                    handle_read_server(read_msg_);
                                    do_read_header();
                                }
                                else
                                {
                                }
                            });
}

void P2pSession::do_write()
{
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
                                boost::asio::buffer(write_msgs_.front().data(),
                                                    write_msgs_.front().length()),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                {
                                    if (!ec)
                                    {
                                        write_msgs_.pop_front();
                                        if (!write_msgs_.empty())
                                        {
                                            do_write();
                                        }
                                    }
                                    else
                                    {
                                    }
                                });
}

//----------------------------------------------------------------------

P2pServer::P2pServer(boost::asio::io_context &io_context, const tcp::endpoint &endpoint)
    : acceptor_(io_context, endpoint)
{
    do_accept();
}

void P2pServer::do_accept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                std::make_shared<P2pSession>(std::move(socket))->start();
            }

            do_accept();
        });
}

//----------------------------------------------------------------------

int P2pNetwork::p2p_server()
{
    try
    {
        boost::asio::io_context io_context;

        tcp::endpoint endpoint(tcp::v4(), std::atoi(PORT));
        P2pServer server(io_context, endpoint);

        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception server: " << e.what() << "\n";
    }

    return 0;
}
