#include "p2p_network.hpp"
#include "p2p_network_c.hpp"

#include "print_or_log.hpp"

using namespace Common;
using namespace Crowd;

bool P2pNetwork::quit_server_ = false;
std::vector<std::string> P2pNetwork::connected_to_server_ = {};
std::vector<std::pair<std::string, std::string>> P2pNetwork::p2p_clients_from_other_thread_ = {};

void P2pNetwork::do_read_header_server(p2p_message read_msg_server)
{
    if (read_msg_server.decode_header() == true)
    {
        do_read_body_server(read_msg_server);
    }
}

void P2pNetwork::do_read_body_server(p2p_message read_msg_server)
{
    handle_read_server(read_msg_server);
}

void P2pNetwork::handle_read_server(p2p_message read_msg_server)
{
    Common::Print_or_log pl;
    if ( !read_msg_server.get_eom_flag()) {
        std::string str_read_msg(read_msg_server.body());
        buf_server_ += str_read_msg.substr(0, read_msg_server.get_body_length());
pl.handle_print_or_log({"___0000444 buf", str_read_msg});
    } else {
        // process json message
        std::string str_read_msg(read_msg_server.body());
        buf_server_ += str_read_msg.substr(0, read_msg_server.get_body_length());
        nlohmann::json buf_j = nlohmann::json::parse(buf_server_);
pl.handle_print_or_log({"___0000444 eom", str_read_msg});

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
        req_conversion["update_you"] =          18;

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
            case 18:    update_you_server(buf_j);
                        break;
            default:    Coin::P2pNetworkC pnc;
                        pnc.handle_read_server_c(buf_j);
                        break;
        }

        buf_server_ = "";
    }
}

void P2pNetwork::register_for_nat_traversal(nlohmann::json buf_j)
{
    nlohmann::json resp_j;
    resp_j["register"] = "ack";

    set_resp_msg_server(resp_j.dump());
    
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Ack for registering client is confirmed"});
}

void P2pNetwork::connect_to_nat(nlohmann::json buf_j)
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

void P2pNetwork::intro_peer(nlohmann::json buf_j)
{
    P2p p2p;
    std::string peer_ip_quad;
    p2p.number_to_ip_string(event_server_.peer->address.host, peer_ip_quad);
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Intro_peer req recv", peer_ip_quad});

    // process buf_j["hash_of_req"] to find ip of the peer who should update you
    std::string co_from_req = buf_j["full_hash_co"];
    std::string email_of_req = buf_j["email_of_req"];
    std::string hash_of_email = buf_j["hash_of_email"];
    // std::string prev_hash_req = buf_j["prev_hash_of_req"];
    std::string ecdsa_pub_key_s = buf_j["ecdsa_pub_key"];
    std::string rsa_pub_key = buf_j["rsa_pub_key"];
    std::string signature = buf_j["signature"];
    std::string req_latest_block = buf_j["latest_block"];
    enet_uint32 req_ip = buf_j["ip"];

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
        std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be

        pl.handle_print_or_log({"______: ", "fh =", prel_first_full_hash_req, "ph =", prel_first_prev_hash_req, email_of_req});

        Rocksy* rocksy = new Rocksy("usersdbreadonly");
        std::string prel_first_coordinator_server = rocksy->FindChosenOne(prel_first_full_hash_req);
        delete rocksy;

        pl.handle_print_or_log({"my_full_hash: ", my_full_hash});
        pl.handle_print_or_log({"prel_first_coordinator: ", prel_first_coordinator_server});

        if (my_full_hash != "" && my_full_hash == prel_first_coordinator_server)
        {
            // pl.handle_print_or_log({"My latest block:", my_latest_block});
            // pl.handle_print_or_log({"Req latest block: ", req_latest_block});

            if (req_latest_block < my_latest_block || req_latest_block == "no blockchain present in folder")
            {
pl.handle_print_or_log({"____00222_0 Is this run?"});
                // Send first block to peer to enable datetime synchonisation
                nlohmann::json first_block_j = proto->get_block_at("0");

                nlohmann::json msg;
                msg["req"] = "send_first_block";
                msg["block"] = first_block_j;
                set_resp_msg_server(msg.dump());
pl.handle_print_or_log({"____00222_2 Is this run?", msg.dump()});
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
            message_j["ip"] = event_server_.peer->address.host;

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
pl.handle_print_or_log({"i: ", std::to_string(i), ", val: ", parts[i]});
                if (i == 1) continue; // ugly hack for a problem in proto.partition_in_buckets()
                if (parts[i] == "") continue; // UGLY hack: "" should be "0"
                
//                 for (auto& element: bm.get_new_users())
//                 {
// pl.handle_print_or_log({"___00 after i:", parts[i], element});
//                     if (parts[i] == element)
//                     {
//                         continue;
//                     }
//                 }

                Rocksy* rocksy = new Rocksy("usersdbreadonly");

                // lookup in rocksdb
                std::string val = parts[i];
                nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
                enet_uint32 ip = value_j["ip"];
                std::string peer_ip;
                P2p p2p;
                p2p.number_to_ip_string(ip, peer_ip);

                std::string req_ip_quad;
                p2p.number_to_ip_string(req_ip, req_ip_quad);

                delete rocksy;

                // if peer ip == this server's ip --> send new_peer to kids
                // --> from_to(my_hash, my_hash) if just me then connected_peers + from_to(my_hash, next hash)
                // --> if more then t.client to same layer co
                // in bucket --> poco --> coord connects to all co --> co connect to other co --> communicate final_hash --> register amount of ok's and nok's

                // inform the underlying network
                if (req_ip_quad == peer_ip) // TODO the else part isn't activated, dunno why, search in test terminals for new_peer
                {
                    // inform server's underlying network
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
                    Rocksy* rocksy = new Rocksy("usersdbreadonly");
                    for (int i = 1; i <= parts_underlying.size(); i++)
                    {
// pl.handle_print_or_log({"___i2: ", i,", parts_underlying: ", parts_underlying[i], ", my_full_hash: ", my_full_hash});
// pl.handle_print_or_log({"i2: ", i, " val2: ", parts_underlying[i]});
                        if (i == 1) continue; // ugly hack for a problem in proto.partition_in_buckets()
                        if (parts_underlying[i] == my_full_hash) continue;

                        // for (auto& element: bm.get_new_users())
                        // {
                        //     if (val == element)
                        //     {
                        //         continue;
                        //     }
                        // }

// pl.handle_print_or_log({"i2: ", i, " parts_underlying: ", parts_underlying[i], ", my_full_hash: ", my_full_hash});
                        // lookup in rocksdb
                        std::string val2 = parts_underlying[i];
                        nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val2));
                        enet_uint32 ip = value_j["ip"];
                        std::string peer_ip_underlying;
                        p2p.number_to_ip_string(ip, peer_ip_underlying);

                        pl.handle_print_or_log({"Send new_peer req: Non-connected underlying peers - client: ", peer_ip_underlying});

                        for (;;)
                        {
                            if (is_connected_to_server(peer_ip_underlying) == false)
                            {
                                break;
                            }
                        }

                        // message to non-connected peers
                        std::string message = message_j.dump();
                        p2p_client(peer_ip_underlying, message);
                    }
                    delete rocksy;
                }

                // inform the other peer's in the same layer (as coordinator)
                pl.handle_print_or_log({"Send new_peer req: Inform my equal layer as coordinator: ", peer_ip});
                
                for (;;)
                {
                    if (is_connected_to_server(peer_ip) == false)
                    {
                        break;
                    }
                }

                std::string message = message_j.dump();
                p2p_client(peer_ip, message);
            }

            // wait 20 seconds or > 1 MB to create block, to process the timestamp if you are the first new_peer request
            intro_msg_vec_.add_to_intro_msg_vec(message_j);

            ip_hemail_vec_.add_ip_hemail_to_ip_hemail_vec(message_j["ip"], hash_of_email); // TODO you have to reset this
        }
        else
        {
            // There's another chosen_one, reply with the correct chosen_one
            pl.handle_print_or_log({"New_co: Chosen_one is someone else!"});

            nlohmann::json message_j;
            message_j["req"] = "new_co";
pl.handle_print_or_log({"prel_first_coordinator_server:______ ", prel_first_coordinator_server});

            Rocksy* rocksy = new Rocksy("usersdbreadonly");
pl.handle_print_or_log({"usersdb size:______ ", (rocksy->TotalAmountOfPeers()).str()});
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(prel_first_coordinator_server));
pl.handle_print_or_log({"______000111_0 does new_co works? ", (value_j["ip"]).dump()});
            enet_uint32 peer_ip = value_j["ip"];
            delete rocksy;
pl.handle_print_or_log({"______000111_1 ", (value_j["ip"]).dump()});
            message_j["ip_co"] = peer_ip;
            set_resp_msg_server(message_j.dump());
pl.handle_print_or_log({"______000111_2 "});
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

void P2pNetwork::new_peer(nlohmann::json buf_j)
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

void P2pNetwork::intro_prel_block(nlohmann::json buf_j)
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
    std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be

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
pl.handle_print_or_log({"___03", "chosen_ones", chosen_ones[i]});
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
pl.handle_print_or_log({"___04", "\n", "mfh", my_full_hash, "\n", "nfh", next_full_hash});
        Crowd::Protocol proto;
std::map<int, std::string> parts11 = proto.partition_in_buckets(my_full_hash, my_full_hash);
int k11;
std::string v11;
for (auto& [k11, v11]: parts11)
{
    pl.handle_print_or_log({"___0400 the whole db", v11});
}
        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, next_full_hash);
pl.handle_print_or_log({"___05"});
        nlohmann::json to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "new_prel_block";
        message_j["latest_block_nr"] = buf_j["latest_block_nr"];
        message_j["block"] = buf_j["block"];
        message_j["prev_hash"] = buf_j["prev_hash"];
        message_j["full_hash_coord"] = buf_j["full_hash_coord"];
pl.handle_print_or_log({"___06"});
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
pl.handle_print_or_log({"___07"});
        Crowd::P2pNetwork pn;
        Poco::Synchronisation sync;
        int key;
        std::string val;
        for (auto &[key, val] : parts)
        {
pl.handle_print_or_log({"___08 introprel chosen_ones", val});
            if (key == 1) continue;
            if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
            if (val == parts[1]) continue; // TODO --> UGLY --> somehow the first and the last chosen_one are the same, you don't need both

            // for (auto& element: bm->get_new_users())
            // {
            //     if (val == element)
            //     {
            //         continue;
            //     }
            // }

pl.handle_print_or_log({"___09 introprel new chosen_ones", val});
            Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");

            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
            uint32_t peer_ip = value_j["ip"];

            delete rocksy;
            
            std::string message = message_j.dump();

            std::string ip_from_peer;
            Crowd::P2p p2p;
            p2p.number_to_ip_string(peer_ip, ip_from_peer);

            pl.handle_print_or_log({"Preparation for new_prel_block: ", ip_from_peer});

            for (;;)
            {
                if (pn.is_connected_to_server(ip_from_peer) == false)
                {
                    break;
                }
            }

            // p2p_client() to all chosen ones with intro_peer request
            pn.p2p_client(ip_from_peer, message);
        }

        delete bm;
pl.handle_print_or_log({"___10"});
    }
    else
    {
        pl.handle_print_or_log({"Intro prel chosen_one is not truthful"});
    }
}

void P2pNetwork::new_prel_block(nlohmann::json buf_j)
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
    std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be

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
pl.handle_print_or_log({"___03", "chosen_ones", chosen_ones[i]});
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
pl.handle_print_or_log({"___04", "\n", "mfh", my_full_hash, "\n", "nfh", next_full_hash});
        Crowd::Protocol proto;
std::map<int, std::string> parts11 = proto.partition_in_buckets(my_full_hash, my_full_hash);
int k11;
std::string v11;
for (auto& [k11, v11]: parts11)
{
    pl.handle_print_or_log({"___0400 the whole db", v11});
}
        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, next_full_hash);
pl.handle_print_or_log({"___05"});
        nlohmann::json to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "new_prel_block";
        message_j["latest_block_nr"] = buf_j["latest_block_nr"];
        message_j["block"] = buf_j["block"];
        message_j["prev_hash"] = buf_j["prev_hash"];
        message_j["full_hash_coord"] = buf_j["full_hash_coord"];
pl.handle_print_or_log({"___06"});
        int k;
        std::string v;
        for (auto &[k, v] : parts)
        {
            message_j["chosen_ones"].push_back(v);
        }
pl.handle_print_or_log({"___07"});
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
pl.handle_print_or_log({"___08"});
        Crowd::P2pNetwork pn;
        Poco::Synchronisation sync;
        int key;
        std::string val;
        for (auto &[key, val] : parts)
        {
pl.handle_print_or_log({"___08 newprel chosen_ones", val});
            if (key == 1) continue;
            if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
            if (val == full_hash_coord) continue;
            if (val == parts[1]) continue; // TODO --> UGLY --> somehow the first and the last chosen_one are the same, you don't need both

            // for (auto& element: bm->get_new_users())
            // {
            //     if (val == element)
            //     {
            //         continue;
            //     }
            // }
            
pl.handle_print_or_log({"___09 newprel new chosen_ones", val});
            Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");

            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
            uint32_t peer_ip = value_j["ip"];

            delete rocksy;
            
            std::string message = message_j.dump();

            std::string ip_from_peer;
            Crowd::P2p p2p;
            p2p.number_to_ip_string(peer_ip, ip_from_peer);

            pl.handle_print_or_log({"Preparation for secondary new_prel_block: ", ip_from_peer});

            for (;;)
            {
                if (pn.is_connected_to_server(ip_from_peer) == false)
                {
                    break;
                }
            }

            // p2p_client() to all chosen ones with intro_peer request
            pn.p2p_client(ip_from_peer, message);
        }

        delete bm;
pl.handle_print_or_log({"___10 end of new_prel_block"});
    }
    else
    {
        pl.handle_print_or_log({"New prel chosen_one is not truthful"});
    }
}

void P2pNetwork::intro_final_block(nlohmann::json buf_j)
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
    std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be

    std::string recv_block_s = recv_block_j.dump();
    Common::Crypto crypto;
    std::string prev_hash_me = crypto.bech32_encode_sha256(recv_block_s);
    Rocksy* rocksy = new Rocksy("usersdbreadonly");
    std::string full_hash_coord_from_me = rocksy->FindChosenOne(prev_hash_me);
    
pl.handle_print_or_log({"________0000000 me", full_hash_coord_from_me,"coord", full_hash_coord_from_coord});
pl.handle_print_or_log({"________0000001 total amount of peers", (rocksy->TotalAmountOfPeers()).str()});
    delete rocksy;


    if (full_hash_coord_from_me == full_hash_coord_from_coord)
    {
        pl.handle_print_or_log({"Coordinator is truthful"});

pl.handle_print_or_log({"block_nr:", recv_latest_block_nr_s});
pl.handle_print_or_log({"block:", recv_block_s});

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

Crowd::Protocol proto;
std::map<int, std::string> partsx = proto.partition_in_buckets(my_full_hash, my_full_hash);
int k1;
std::string v1;
for (auto &[k1, v1] : partsx)
{
    pl.handle_print_or_log({"___000110 intro_final_block", std::to_string(k1), v1});
}

//Protocol proto;
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

        for (int i = 0; i < chosen_ones.size(); i++)
        {
            if (i < j)
            {
pl.handle_print_or_log({"___000_"});
                if (chosen_ones[i] == buf_j["full_hash_coord"]) continue;

                std::string c_one = chosen_ones[i];
pl.handle_print_or_log({"___00 c_one", c_one});
                Rocksy* rocksy3 = new Rocksy("usersdbreadonly");
                nlohmann::json value_j = nlohmann::json::parse(rocksy3->Get(c_one));
//pl.handle_print_or_log({"___01 value_j", value_j.dump()});
                delete rocksy3;

                enet_uint32 ip = value_j["ip"];
                std::string peer_ip;
                P2p p2p;
                p2p.number_to_ip_string(ip, peer_ip);
pl.handle_print_or_log({"___02"});
pl.handle_print_or_log({"___02 peer_ip", peer_ip});
                nlohmann::json msg_j;
                msg_j["req"] = "hash_comparison";
                msg_j["hash_comp"] = prev_hash_me == prev_hash_from_saved_block_at_place_i;
                std::string msg_s = msg_j.dump();

                p2p_client(peer_ip, msg_s);
            }
            else if (i == j)
            {
                continue;
            }
        }
pl.handle_print_or_log({"___03"});
        // inform your underlying ones from this block
        std::string next_full_hash;
        for (int i = 0; i < chosen_ones.size(); i++)
        {
pl.handle_print_or_log({"___03", "chosen_ones", chosen_ones[i]});
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
pl.handle_print_or_log({"___04", "\n", "mfh", my_full_hash, "\n", "nfh", next_full_hash});

std::map<int, std::string> parts11 = proto.partition_in_buckets(my_full_hash, my_full_hash);
int k11;
std::string v11;
for (auto& [k11, v11]: parts11)
{
    pl.handle_print_or_log({"___0400 the whole db", v11});
}

        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, next_full_hash);
pl.handle_print_or_log({"___05"});
        nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "new_final_block";
        message_j["latest_block_nr"] = buf_j["latest_block_nr"];
        message_j["block"] = buf_j["block"];
        message_j["full_hash_coord"] = buf_j["full_hash_coord"];
        message_j["rocksdb"] = buf_j["rocksdb"];
pl.handle_print_or_log({"___06"});
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
pl.handle_print_or_log({"___07"});
        int key;
        std::string val;
        Poco::BlockMatrix bm;
        for (auto &[key, val] : parts)
        {
pl.handle_print_or_log({"___08 introblock chosen_ones", val});
            if (key == 1) continue;
            if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
            if (val == full_hash_coord_from_coord) continue;
            if (val == parts[1]) continue; // TODO --> UGLY --> somehow the first and the last chosen_one are the same, you don't need both

            // for (auto& element: bm.get_new_users())
            // {
            //     if (val == element)
            //     {
            //         continue;
            //     }
            // }

pl.handle_print_or_log({"___09 introblock new chosen_ones", val});
            Crowd::Rocksy* rocksy4 = new Crowd::Rocksy("usersdbreadonly");

            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy4->Get(val));
            uint32_t peer_ip = value_j["ip"];

            delete rocksy4;
            
            std::string message = message_j.dump();

            std::string ip_from_peer;
            Crowd::P2p p2p;
            p2p.number_to_ip_string(peer_ip, ip_from_peer);

            pl.handle_print_or_log({"Preparation for new_final_block:", ip_from_peer});

            for (;;)
            {
                if (is_connected_to_server(ip_from_peer) == false)
                {
                    break;
                }
            }

            // p2p_client() to all chosen ones with intro_peer request
            p2p_client(ip_from_peer, message);
        }
pl.handle_print_or_log({"___10"});
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

void P2pNetwork::new_final_block(nlohmann::json buf_j)
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

pl.handle_print_or_log({"block_nr:", recv_latest_block_nr_s});
pl.handle_print_or_log({"block:", recv_block_j.dump()});

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
std::string my_full_hash = fh.get_full_hash_from_file();
Crowd::Protocol proto;
std::map<int, std::string> partsx = proto.partition_in_buckets(my_full_hash, my_full_hash);
int k1;
std::string v1;
for (auto &[k1, v1] : partsx)
{
    pl.handle_print_or_log({"___000111 new_final_block", std::to_string(k1), v1});
}

// FullHash fh;
// std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be

    std::string recv_block_s = recv_block_j.dump();
    Common::Crypto crypto;
    std::string prev_hash_me = crypto.bech32_encode_sha256(recv_block_s);

//Protocol protocol;
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

    for (int i = 0; i < chosen_ones.size(); i++)
    {
        if (i < j)
        {
            if (chosen_ones[i] == buf_j["full_hash_coord"]) continue;

            std::string c_one = chosen_ones[i];
            Rocksy* rocksy2 = new Rocksy("usersdbreadonly");
            nlohmann::json value_j = nlohmann::json::parse(rocksy2->Get(c_one));
            delete rocksy2;

            enet_uint32 ip = value_j["ip"];
            std::string peer_ip;
            P2p p2p;
            p2p.number_to_ip_string(ip, peer_ip);

            nlohmann::json msg_j;
            msg_j["req"] = "hash_comparison";
            msg_j["hash_comp"] = prev_hash_me == prev_hash_from_saved_block_at_place_i;
            std::string msg_s = msg_j.dump();

            p2p_client(peer_ip, msg_s);
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
pl.handle_print_or_log({"___04", "\n", "mfh", my_full_hash, "\n", "nfh", next_full_hash});

std::map<int, std::string> parts11 = proto.partition_in_buckets(my_full_hash, my_full_hash);
int k11;
std::string v11;
for (auto& [k11, v11]: parts11)
{
    pl.handle_print_or_log({"___0400 the whole db", v11});
}

    std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, next_full_hash);
pl.handle_print_or_log({"___05"});

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

    Crowd::P2pNetwork pn;
    int key;
    std::string val;
    Poco::BlockMatrix bm;
    for (auto &[key, val] : parts)
    {
pl.handle_print_or_log({"___08 newblock chosen_ones", val});
        if (key == 1) continue;
        if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
        if (val == full_hash_coord_from_coord) continue;
        if (val == parts[1]) continue; // TODO --> UGLY --> somehow the first and the last chosen_one are the same, you don't need both

        // for (auto& element: bm.get_new_users())
        // {
        //     if (val == element)
        //     {
        //         continue;
        //     }
        // }
        
pl.handle_print_or_log({"___09 newblock new chosen_ones", val});
        Crowd::Rocksy* rocksy3 = new Crowd::Rocksy("usersdbreadonly");

        // lookup in rocksdb
        nlohmann::json value_j = nlohmann::json::parse(rocksy3->Get(val));
        uint32_t peer_ip = value_j["ip"];

        delete rocksy3;
        
        std::string message = message_j.dump();

        std::string ip_from_peer;
        Crowd::P2p p2p;
        p2p.number_to_ip_string(peer_ip, ip_from_peer);

        pl.handle_print_or_log({"Preparation for new_final_block:", ip_from_peer});

        for (;;)
        {
            if (pn.is_connected_to_server(ip_from_peer) == false)
            {
                break;
            }
        }

        // p2p_client() to all chosen ones with intro_peer request
        pn.p2p_client(ip_from_peer, message);
    }
}

void P2pNetwork::your_full_hash(nlohmann::json buf_j)
{
    // my full hash
    std::string full_hash = buf_j["full_hash"];
    std::string prev_hash = buf_j["prev_hash"];
    Common::Print_or_log pl;
    pl.handle_print_or_log({"New peer's full_hash (server):", full_hash});
    pl.handle_print_or_log({"New peer's prev_hash (server):", prev_hash});

    // save full_hash
    FullHash fh;
    fh.save_full_hash_to_file(full_hash);

    // save prev_hash
    PrevHash ph;
    ph.save_my_prev_hash_to_file(prev_hash);
    
    nlohmann::json block_j = buf_j["block"];
    std::string req_latest_block_nr = buf_j["block_nr"];
// pl.handle_print_or_log({"block_nr:", req_latest_block_nr});
// pl.handle_print_or_log({"block:", block_j.dump()});

    // Update my blocks, rocksdb and matrices
    Protocol proto;
    std::string my_latest_block = proto.get_last_block_nr();
    nlohmann::json m_j;
    m_j["req"] = "update_me";
    m_j["block_nr"] = my_latest_block;
    set_resp_msg_server(m_j.dump());
}

void P2pNetwork::hash_comparison(nlohmann::json buf_j)
{
    // compare the received hash
    Common::Print_or_log pl;
    pl.handle_print_or_log({"The hash comparison is (server): ",  (buf_j["hash_comp"]).dump()});

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());
}

void P2pNetwork::intro_online(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"intro new peer online: ", (buf_j["full_hash"]).dump()});
    
    nlohmann::json to_verify_j;
    to_verify_j["req"] = buf_j["req"];
    to_verify_j["full_hash"] = buf_j["full_hash"];
    to_verify_j["latest_block_nr"] = buf_j["latest_block_nr"];
    to_verify_j["ip"] = buf_j["ip"];
    to_verify_j["server"] = buf_j["server"];
    to_verify_j["fullnode"] = buf_j["fullnode"];

    Rocksy* rocksy = new Rocksy("usersdbreadonly");
    std::string full_hash = buf_j["full_hash"];
    nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(full_hash));
    std::string ecdsa_pub_key_s = contents_j["ecdsa_pub_key"];
    delete rocksy;

    Crypto* crypto = new Crypto();
    std::string to_verify_s = to_verify_j.dump();
    ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
    crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
    std::string signature = buf_j["signature"];
    std::string signature_bin = crypto->base64_decode(signature);
    
    if (crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
    {
        pl.handle_print_or_log({"verified new online user"});

        // inform the network of new online presence
        Protocol proto;
        FullHash fh;
        std::string my_full_hash = fh.get_full_hash_from_file();

        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

        nlohmann::json message_j, to_sign_j;
        message_j["req"] = "new_online";
        message_j["full_hash"] = full_hash;
        message_j["ip"] = buf_j["ip"];
        message_j["server"] = buf_j["server"];
        message_j["fullnode"] = buf_j["fullnode"];

        int k;
        std::string v;
        for (auto &[k, v] : parts)
        {
            message_j["chosen_ones"].push_back(v);
        }

        to_sign_j["req"] = message_j["req"];
        to_sign_j["full_hash"] = full_hash;
        to_sign_j["ip"] = buf_j["ip"];
        to_sign_j["server"] = buf_j["server"];
        to_sign_j["fullnode"] = buf_j["fullnode"];
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

        Rocksy* rocksy = new Rocksy("usersdb");

        int key;
        std::string val;
        Poco::BlockMatrix bm;
        for (auto &[key, val] : parts)
        {
            if (key == 1) continue;
            if (val == full_hash) continue;
            if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
            if (val == parts[1]) continue; // TODO --> UGLY --> somehow the first and the last chosen_one are the same, you don't need both
            
            // for (auto& element: bm.get_new_users())
            // {
            //     if (val == element)
            //     {
            //         continue;
            //     }
            // }
            
            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
            uint32_t peer_ip = value_j["ip"];
            
            std::string ip_from_peer;
            P2p p2p;
            p2p.number_to_ip_string(peer_ip, ip_from_peer);

            pl.handle_print_or_log({"Preparation for new_online:", ip_from_peer});

            for (;;)
            {
                if (is_connected_to_server(ip_from_peer) == false)
                {
                    break;
                }
            }

            // p2p_client() to all chosen ones with intro_peer request
            p2p_client(ip_from_peer, message_s);
        }

        // update this rocksdb
        nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(full_hash));
        value_j["online"] = true;
        value_j["ip"] = buf_j["ip"];
        value_j["server"] = buf_j["server"];
        value_j["fullnode"] = buf_j["fullnode"];
        std::string value_s = value_j.dump();
        rocksy->Put(full_hash, value_s);

        // update new user's blockchain and rocksdb
        std::string my_latest_block = proto.get_last_block_nr();
        std::string req_latest_block = buf_j["latest_block_nr"];
        
        if (req_latest_block < my_latest_block || req_latest_block == "no blockchain present in folder")
        {
            // TODO: upload blockchain to the requester starting from latest block
            // Update blockchain: send latest block to peer
            nlohmann::json list_of_blocks_j = proto.get_blocks_from(req_latest_block);
            //pl.handle_print_or_log({"list_of_blocks_s:", list_of_blocks_j.dump()});
            uint64_t value;
            std::istringstream iss(my_latest_block);
            iss >> value;

            for (uint64_t i = 0; i <= value; i++)
            {
                nlohmann::json block_j = list_of_blocks_j[i]["block"];
                //pl.handle_print_or_log({"block_j:", block_j.dump()});
                nlohmann::json msg;
                msg["req"] = "update_your_blocks";
                std::ostringstream o;
                o << i;
                msg["block_nr"] = o.str();
                msg["block"] = block_j;
                set_resp_msg_server(msg.dump());
            }

            // Update rockdb's:
            // How to? Starting from the blocks? Lookup all users in the blocks starting from a block
            // , then lookup those user_id's in rocksdb and send
            nlohmann::json list_of_users_j = nlohmann::json::parse(proto.get_all_users_from(req_latest_block)); // TODO: there are double parse/dumps everywhere
                                                                                                                // maybe even a stack is better ...
            for (auto& user : list_of_users_j) // TODO better make a map of all keys with its values and send that once
            {
                nlohmann::json msg;
                msg["req"] = "update_your_rocksdb";
                msg["key"] = user;

                std::string u = user;
                std::string value = rocksy->Get(u);
                msg["value"] = value;

                set_resp_msg_server(msg.dump());
            }
        }

        delete rocksy;
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

void P2pNetwork::new_online(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"new peer online:", (buf_j["full_hash"]).dump(), ", inform your bucket"});

    Protocol proto;
    FullHash fh;
    std::string my_full_hash = fh.get_full_hash_from_file();
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

    nlohmann::json message_j, to_sign_j;
    message_j["req"] = "new_online";
    message_j["full_hash"] = buf_j["full_hash"];
    message_j["ip"] = buf_j["ip"];
    message_j["server"] = buf_j["server"];
    message_j["fullnode"] = buf_j["fullnode"];

    int k;
    std::string v;
    for (auto &[k, v] : parts)
    {
        message_j["chosen_ones"].push_back(v);
    }

    to_sign_j["req"] = message_j["req"];
    to_sign_j["full_hash"] = buf_j["full_hash"];
    to_sign_j["ip"] = buf_j["ip"];
    to_sign_j["server"] = buf_j["server"];
    to_sign_j["fullnode"] = buf_j["fullnode"];
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

    std::string full_hash = buf_j["full_hash"];

    Rocksy* rocksy = new Rocksy("usersdb");
    
    int key;
    std::string val;
    Poco::BlockMatrix bm;
    for (auto &[key, val] : parts)
    {
        if (key == 1) continue;
        if (val == full_hash) continue;
        if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
        if (val == parts[1]) continue; // TODO --> UGLY --> somehow the first and the last chosen_one are the same, you don't need both
        
        // for (auto& element: bm.get_new_users())
        // {
        //     if (val == element)
        //     {
        //         continue;
        //     }
        // }

        // lookup in rocksdb
        nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
        uint32_t peer_ip = value_j["ip"];
        
        std::string message_s = message_j.dump();

        std::string ip_from_peer;
        P2p p2p;
        p2p.number_to_ip_string(peer_ip, ip_from_peer);

        pl.handle_print_or_log({"Preparation for new_online: ", ip_from_peer});

        for (;;)
        {
            if (is_connected_to_server(ip_from_peer) == false)
            {
                break;
            }
        }

        // p2p_client() to all chosen ones with intro_peer request
        p2p_client(ip_from_peer, message_s);
    }

    // update this rocksdb
    nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(full_hash));
    value_j["online"] = true;
    value_j["ip"] = buf_j["ip"];
    value_j["server"] = buf_j["server"];
    value_j["fullnode"] = buf_j["fullnode"];
    std::string value_s = value_j.dump();
    rocksy->Put(full_hash, value_s);

    delete rocksy;
}

void P2pNetwork::update_you_server(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Update_me: receive all blocks, rocksdb and matrices from server (server)"});

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());

    // Update blocks
    nlohmann::json blocks_j = buf_j["blocks"];
    for (auto& b: blocks_j.items())
    {
        nlohmann::json block_j = b.value()["block"];
        std::string block_nr = b.value()["block_nr"];

        merkle_tree mt;
        mt.save_block_to_file(block_j, block_nr);
    }
pl.handle_print_or_log({"__00_s"});
    // Update rocksdb
    nlohmann::json rdb_j = buf_j["rocksdb"];
pl.handle_print_or_log({"__00_s", rdb_j.dump()});
    for (auto& element : rdb_j)
    {
        std::string key_s = element["full_hash"];
        std::string value_s = element.dump();
pl.handle_print_or_log({"__00_s k", key_s});
pl.handle_print_or_log({"__00_s v", value_s});
        Rocksy* rocksy = new Rocksy("usersdb");
        rocksy->Put(key_s, value_s);
        delete rocksy;
    }

FullHash fh;
std::string my_full_hash = fh.get_full_hash_from_file();
Crowd::Protocol proto;
std::map<int, std::string> partsx = proto.partition_in_buckets(my_full_hash, my_full_hash);
int k;
std::string v;
for (auto &[k, v] : partsx)
{
    pl.handle_print_or_log({"___000112 update_you_server", std::to_string(k), v});
}

pl.handle_print_or_log({"__01_s"});
    
    // Update matrices
    nlohmann::json block_matrix_j = buf_j["bm"];
    nlohmann::json intro_msg_s_matrix_j = buf_j["imm"];
    nlohmann::json ip_all_hashes_j = buf_j["iah"];

    Poco::BlockMatrix bm;
    Poco::IntroMsgsMat imm;
    Poco::IpAllHashes iah;
pl.handle_print_or_log({"__02_s"});
    bm.get_block_matrix().clear(); // TODO clear the matrix --> this doesn't clear it
    bm.get_calculated_hash_matrix().clear();
    bm.get_prev_hash_matrix().clear();
pl.handle_print_or_log({"__03_s"});
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
pl.handle_print_or_log({"__04_s"});
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
pl.handle_print_or_log({"__05_s"});
    for (auto& [k1, v1] : ip_all_hashes_j.items())
    {
        for (auto& [k2, v2] : v1.items())
        {
            for (auto& [k3, v3] : v2.items())
            {
                std::pair<enet_uint32, std::string> myPair = std::make_pair(v3["first"], v3["second"]);
                std::shared_ptr<std::pair<enet_uint32, std::string>> ptr(new std::pair<enet_uint32, std::string> (myPair));
                iah.add_ip_hemail_to_ip_all_hashes_vec(ptr);
            }

            iah.add_ip_all_hashes_vec_to_ip_all_hashes_2d_mat();
        }

        iah.add_ip_all_hashes_2d_mat_to_ip_all_hashes_3d_mat();
    }

    // Update intro_msg_vec and ip_hemail_vec
    nlohmann::json intro_msg_vec_j = buf_j["imv"];
    nlohmann::json ip_hemail_vec_j = buf_j["ihv"];

    for (auto& el: intro_msg_vec_j)
    {
        intro_msg_vec_.add_to_intro_msg_vec(el);
    }

    for (auto& [k, v]: ip_hemail_vec_j.items())
    {
        enet_uint32 key = stoi(k);
        ip_hemail_vec_.add_ip_hemail_to_ip_hemail_vec(key, v);
    }

// // for debugging purposes:
// auto block_matrix = bm.get_block_matrix();
// for (int i = 0; i < block_matrix.size(); i++)
// {
//     for (int j = 0; j < block_matrix.at(i).size(); j++)
//     {
//         nlohmann::json content_j = *block_matrix.at(i).at(j);
//         pl.handle_print_or_log({"___00block matrix entries", std::to_string(i), "update", std::to_string(j), "(oldest first)", content_j.dump()});
//     }
// }
}

void P2pNetwork::set_resp_msg_server(std::string msg)
{
    std::vector<std::string> splitted = split(msg, p2p_message::max_body_length);
    p2p_message resp_msg_server;
    for (int i = 0; i < splitted.size(); i++)
    {
        char s[p2p_message::max_body_length];
        strncpy(s, splitted[i].c_str(), sizeof(s));

        resp_msg_server.body_length(std::strlen(s));
        std::memcpy(resp_msg_server.body(), s, resp_msg_server.body_length());
        i == splitted.size() - 1 ? resp_msg_server.encode_header(1) : resp_msg_server.encode_header(0); // 1 indicates end of message eom, TODO perhaps a set_eom_flag(true) instead of an int

        // sprintf(buffer_, "%s", (char*) resp_j.dump());
        packet_server_ = enet_packet_create(resp_msg_server.data(), strlen(resp_msg_server.data())+1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(event_server_.peer, 0, packet_server_);
        enet_host_flush(server_);
    }
}

int P2pNetwork::p2p_server()
{
    int  i;

    Common::Print_or_log pl;

    if (enet_initialize() != 0)
    {
        pl.handle_print_or_log({"Could not initialize enet."});
        return 0;
    }

    address_server_.host = ENET_HOST_ANY;
    address_server_.port = PORT;

    server_ = enet_host_create(&address_server_, 210, 2, 0, 0);

    if (server_ == NULL)
    {
        pl.handle_print_or_log({"Could not start server."});
        return 0;
    }

    Crowd::P2p p2p;
    std::string connected_peer;
    p2p_message read_msg_server;
    while (true)
    {
        if (get_quit_server_req() == true) break;

        int er = enet_host_service(server_, &event_server_, 50);
        while (er > 0)
        //while (enet_host_service(server_, &event_server_, 50) > 0)
        {
            if (get_quit_server_req() == true) break;

            // process event
            switch (event_server_.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    p2p.number_to_ip_string(event_server_.peer->address.host, connected_peer);
                    add_to_connected_to_server(connected_peer);
                    pl.handle_print_or_log({"Incoming peer.", connected_peer});

for (auto& el: get_connected_to_server())
{
    pl.handle_print_or_log({"___0009876 peers all", el});
}

                    break;

                case ENET_EVENT_TYPE_RECEIVE:
pl.handle_print_or_log({"___0009876 receive"});
                    sprintf(read_msg_server.data(), "%s", (char*) event_server_.packet->data);
                    do_read_header_server(read_msg_server);
                    enet_packet_destroy(event_server_.packet);

                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    p2p.number_to_ip_string(event_server_.peer->address.host, connected_peer);
                    remove_from_connected_to_server(connected_peer);
                    pl.handle_print_or_log({"Peer has disconnected.", connected_peer});

for (auto& el: get_connected_to_server())
{
    pl.handle_print_or_log({"___0009877 peers all", el});
}

                    free(event_server_.peer->data);
                    event_server_.peer->data = NULL;

                    break;

                default:
                    pl.handle_print_or_log({"Tick tock."});
                    break;
            }

            // in another the p2p_client calls stall when testing
            do_p2p_clients_from_other_thread();

            er = enet_host_check_events(server_, &event_server_);
        }
    }

    enet_host_destroy(server_);
    enet_deinitialize();

    return 1;
}

