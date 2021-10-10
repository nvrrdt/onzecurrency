#include "p2p_network.hpp"
#include "p2p_network_c.hpp"

using namespace Common;
using namespace Crowd;

bool P2pNetwork::quit_server_ = false;

void P2pNetwork::do_read_header_server()
{
    if (read_msg_.decode_header() == true)
    {
        do_read_body_server();
    }
}

void P2pNetwork::do_read_body_server()
{
    handle_read_server();
}

void P2pNetwork::handle_read_server()
{
    if ( !read_msg_.get_eom_flag()) {
        std::string str_read_msg(read_msg_.body());
        buf_server_ += str_read_msg.substr(0, read_msg_.get_body_length());
        
    } else {
        // process json message
        std::string str_read_msg(read_msg_.body());
        buf_server_ += str_read_msg.substr(0, read_msg_.get_body_length());
        nlohmann::json buf_j = nlohmann::json::parse(buf_server_);

        std::string req = buf_j["req"];
        std::map<std::string, int> req_conversion;
        req_conversion["register"] =            1;
        req_conversion["connect"] =             2;
        req_conversion["intro_peer"] =          3;
        req_conversion["new_peer"] =            4;
        req_conversion["update_your_blocks"] =  5;
        req_conversion["update_your_rocksdb"] = 6;
        req_conversion["intro_prel_block"] =    7;
        req_conversion["new_prel_block"] =      8;
        req_conversion["intro_final_block"] =   9;
        req_conversion["new_final_block"] =     10;
        req_conversion["your_full_hash"] =      11;
        req_conversion["update_my_matrices"] =  12;
        req_conversion["hash_comparison"] =     13;
        req_conversion["update_my_blocks_and_rocksdb"] = 14;
        req_conversion["update_your_matrices"] =    15;
        req_conversion["intro_online"] =        16;
        req_conversion["new_online"] =          17;

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
            case 5:     update_your_blocks(buf_j);
                        break;
            case 6:     update_your_rocksdb(buf_j);
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
            case 12:    update_my_matrices(buf_j);
                        break;
            case 13:    hash_comparison(buf_j);
                        break;
            case 14:    update_my_blocks_and_rocksdb(buf_j);
                        break;
            case 15:    update_your_matrices_server(buf_j);
                        break;
            case 16:    intro_online(buf_j);
                        break;
            case 17:    new_online(buf_j);
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
    
    std::cout << "Ack for registering client is confirmed" << std::endl;
}

void P2pNetwork::connect_to_nat(nlohmann::json buf_j)
{
    std::cout << "connection request for a peer behind a nat" << std::endl;

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

    if (req_ip == event_.peer->address.host)
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
        std::cout << "Email validated and message verified" << std::endl;

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
std::cout << "______: " << prel_first_prev_hash_req << " , " << email_of_req << " , " << prel_first_full_hash_req << std::endl;

        Rocksy* rocksy = new Rocksy("usersdb");
        std::string prel_first_coordinator_server = rocksy->FindChosenOne(prel_first_full_hash_req);
        delete rocksy;
        
        FullHash fh;
        std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be
        
        if (my_full_hash != "" && my_full_hash == prel_first_coordinator_server)
        {
            std::cout << "my_full_hash: " << my_full_hash << std::endl;

            Protocol proto;
            std::string my_latest_block = proto.get_last_block_nr();
            // std::cout << "My latest block: " << my_latest_block << std::endl;
            // std::cout << "Req latest block: " << req_latest_block << std::endl;

            if (req_latest_block < my_latest_block || req_latest_block == "no blockchain present in folder")
            {
                // Send first block to peer to enable datetime synchonisation
                nlohmann::json first_block_j = proto.get_block_at("0");

                nlohmann::json msg;
                msg["req"] = "send_first_block";
                msg["block"] = first_block_j;
                set_resp_msg_server(msg.dump());
            }

            // Disconect from client
            nlohmann::json msg_j;
            msg_j["req"] = "close_this_conn_and_create";
            set_resp_msg_server(msg_j.dump());

            std::cout << "1 or more totalamountofpeers! " << std::endl;

            // communicate intro_peers to chosen_one's with a new_peer req

            std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

            nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
            message_j["req"] = "new_peer";
            message_j["email_of_req"] = email_of_req;
            message_j["hash_of_email"] = hash_of_email;
            message_j["full_hash_co"] = my_full_hash;
            message_j["ecdsa_pub_key"] = ecdsa_pub_key_s;
            message_j["rsa_pub_key"] = rsa_pub_key;
            message_j["ip"] = event_.peer->address.host;

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

            std::string key, val;
            for (int i = 1; i <= parts.size(); i++)
            {
//std::cout << "i: " << i << ", val: " << parts[i] << std::endl;
                if (i == 1) continue; // ugly hack for a problem in proto.partition_in_buckets()
                if (parts[i] == "") continue; // UGLY hack: "" should be "0"
                
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
                    std::cout << "Send new_peer req: Inform my underlying network as coordinator" << std::endl;

                    std::string next_hash = parts[2];
                    std::map<int, std::string> parts_underlying = proto.partition_in_buckets(my_full_hash, next_hash);
                    std::string key2, val2;
                    Rocksy* rocksy = new Rocksy("usersdbreadonly");
                    for (int i = 1; i <= parts_underlying.size(); i++)
                    {
//std::cout << "___i2: " << i << ", parts_underlying: " << parts_underlying[i] << ", my_full_hash: " << my_full_hash << std::endl;
//std::cout << "i2: " << i << " val2: " << parts_underlying[i] << std::endl;
                        if (i == 1) continue; // ugly hack for a problem in proto.partition_in_buckets()
                        if (parts_underlying[i] == my_full_hash) continue;
//std::cout << "i2: " << i << " parts_underlying: " << parts_underlying[i] << ", my_full_hash: " << my_full_hash << std::endl;
                        // lookup in rocksdb
                        std::string val2 = parts_underlying[i];
                        nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val2));
                        enet_uint32 ip = value_j["ip"];
                        std::string peer_ip_underlying;
                        p2p.number_to_ip_string(ip, peer_ip_underlying);

                        std::cout << "Send new_peer req: Non-connected underlying peers - client: " << peer_ip_underlying << std::endl;
                        // message to non-connected peers
                        std::string message = message_j.dump();
                        p2p_client(peer_ip_underlying, message);
                    }
                    delete rocksy;
                }
                else
                {
                    if (peer_ip == "") continue;

                    // inform the other peer's in the same layer (as coordinator)
                    std::cout << "Send new_peer req: Inform my equal layer as coordinator: " << peer_ip << std::endl;
                    
                    std::string message = message_j.dump();
                    p2p_client(peer_ip, message);
                }
            }

            // wait 20 seconds or > 1 MB to create block, to process the timestamp if you are the first new_peer request
            intro_msg_vec_.add_to_intro_msg_vec(message_j);

            ip_hemail_vec_.add_ip_hemail_to_ip_hemail_vec(message_j["ip"], hash_of_email); // TODO you have to reset this
        }
        else
        {
            // There's another chosen_one, reply with the correct chosen_one
            std::cout << "Chosen_one is someone else!" << std::endl;

            nlohmann::json message_j;
            message_j["req"] = "new_co";
std::cout << "prel_first_coordinator_server:______ " << prel_first_coordinator_server << std::endl;

            Rocksy* rocksy = new Rocksy("usersdb");
std::cout << "usersdb size:______ " << rocksy->TotalAmountOfPeers() << std::endl;
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(prel_first_coordinator_server));
            enet_uint32 peer_ip = value_j["ip"];
            delete rocksy;

            message_j["ip_co"] = peer_ip;
            set_resp_msg_server(message_j.dump());
        }
        
        delete crypto;
    }
    else
    {
        std::cout << "Email incorrect and/or message verification incorrect" << std::endl;
    }

    
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
    std::cout << "New_peer req recv: " << std::endl;
    // should read the timestamp of the first new_peer request received
    
    // wait 20 seconds or > 1 MB to create block, to process the timestamp if you are the first new_peer request
    intro_msg_vec_.add_to_intro_msg_vec(buf_j);

    ip_hemail_vec_.add_ip_hemail_to_ip_hemail_vec(buf_j["ip"], buf_j["hash_of_email"]); // TODO you have to reset this

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());
}

void P2pNetwork::update_your_blocks(nlohmann::json buf_j)
{
    std::cout << "update_your_blocks server" << std::endl;
    // save blocks to blockchain folder

    nlohmann::json block_j = buf_j["block"];
    std::string block_nr = buf_j["block_nr"];
    // std::cout << "block_s: " << block_j.dump() << std::endl;
    // std::cout << "block_nr: " << block_nr << std::endl;

    merkle_tree mt;
    mt.save_block_to_file(block_j, block_nr);
}

void P2pNetwork::update_your_rocksdb(nlohmann::json buf_j)
{
    std::cout << "update_your_rocksdb server" << std::endl;

    std::string key_s = buf_j["key"];
    std::string value_s = buf_j["value"];

    Rocksy* rocksy = new Rocksy("usersdb");
    rocksy->Put(key_s, value_s);
    delete rocksy;
}

void P2pNetwork::intro_prel_block(nlohmann::json buf_j)
{
    // intro_prel_block
    std::cout << "Intro_prel_block: " << std::endl;

    // communicate preliminary block to other peers --> needs to be calculated depending on this server's place in the chosen_ones list

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());

    // add received_blocks to received_block_vector
    nlohmann::json recv_block_j = buf_j["block"];
    Poco::BlockMatrix *bm = new Poco::BlockMatrix();
    bm->add_received_block_to_received_block_vector(recv_block_j);
    delete bm;

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
        std::cout << "Intro prel chosen_one is truthful" << std::endl;

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

        Crowd::Protocol proto;
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

        Crowd::P2pNetwork pn;
        Poco::Synchronisation sync;
        std::string key, val;
        for (auto &[key, val] : parts)
        {
            if (sync.get_break_block_creation_loops()) break;
            
            if (key == 1) continue;
            if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other

            Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdb");

            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
            uint32_t peer_ip = value_j["ip"];

            delete rocksy;
            
            std::string message = message_j.dump();

            std::cout << "Preparation for prel_new_block: " << peer_ip << std::endl;

            std::string ip_from_peer;
            Crowd::P2p p2p;
            p2p.number_to_ip_string(peer_ip, ip_from_peer);

            // p2p_client() to all chosen ones with intro_peer request
            pn.p2p_client(ip_from_peer, message);
        }
    }
    else
    {
        std::cout << "Intro prel chosen_one is not truthful" << std::endl;
    }
}

void P2pNetwork::new_prel_block(nlohmann::json buf_j)
{
    // new_prel_block --> TODO block should be saved
    std::cout << "New_prel_block: " << std::endl;

    // communicate preliminary block to other peers --> needs to be calculated depending on this server's place in the chosen_ones list

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());

    // add received_blocks to received_block_vector
    nlohmann::json recv_block_j = buf_j["block"];
    Poco::BlockMatrix *bm = new Poco::BlockMatrix();
    bm->add_received_block_to_received_block_vector(recv_block_j);
    delete bm;

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
        std::cout << "New prel chosen_one is truthful" << std::endl;

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

        Crowd::Protocol proto;
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

        Crowd::P2pNetwork pn;
        Poco::Synchronisation sync;
        std::string key, val;
        for (auto &[key, val] : parts)
        {
            if (sync.get_break_block_creation_loops()) break;
            
            if (key == 1) continue;
            if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
            if (val == full_hash_coord) continue;
            
            Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdb");

            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
            uint32_t peer_ip = value_j["ip"];

            delete rocksy;
            
            std::string message = message_j.dump();

            std::cout << "Preparation for secondary prel_new_block: " << peer_ip << std::endl;

            std::string ip_from_peer;
            Crowd::P2p p2p;
            p2p.number_to_ip_string(peer_ip, ip_from_peer);

            // p2p_client() to all chosen ones with intro_peer request
            pn.p2p_client(ip_from_peer, message);
        }
    }
    else
    {
        std::cout << "New prel chosen_one is not truthful" << std::endl;
    }
}

void P2pNetwork::intro_final_block(nlohmann::json buf_j)
{
    // intro_final_block
    std::cout << "Intro_final_block: " << std::endl;

    // Is the coordinator the truthful final coordinator for this block?
    // Compare the hashes from the block of the coordinator with your saved blocks' hashes
    // communicate comparison to other chosen_ones --> needs to be calculated depending on this server's place in the chosen_ones list
    // Then inform your underlying network

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());

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
    Rocksy* rocksy = new Rocksy("usersdb");
    std::string full_hash_coord_from_me = rocksy->FindChosenOne(prev_hash_me);
    delete rocksy;

    if (full_hash_coord_from_me == full_hash_coord_from_coord)
    {
        std::cout << "Coordinator is truthful" << std::endl;

std::cout << "block_nr: " << recv_latest_block_nr_s << std::endl;
std::cout << "block: " << recv_block_s << std::endl;

        // Save block
        merkle_tree mt;
        mt.save_block_to_file(recv_block_j, recv_latest_block_nr_s); // TODO what about parallel blocks?

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

        Protocol protocol;
        nlohmann::json saved_block_at_place_i = protocol.get_block_at(recv_latest_block_nr_s);
        std::string saved_block_at_place_i_s = saved_block_at_place_i.dump();
        std::string prev_hash_from_saved_block_at_place_i = crypto.bech32_encode_sha256(saved_block_at_place_i_s);

        bool comparison = prev_hash_me == prev_hash_from_saved_block_at_place_i;
        std::cout << "Comparison is: " << comparison << std::endl;

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
std::cout << "___000_ " << std::endl;
                if (chosen_ones[i] == buf_j["full_hash_coord"]) continue;

                std::string c_one = chosen_ones[i];
std::cout << "___00 c_one " << c_one << std::endl;
                Rocksy* rocksy = new Rocksy("usersdbreadonly");
                nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(c_one));
std::cout << "___01 value_j " << value_j.dump() << std::endl;
                delete rocksy;

                enet_uint32 ip = value_j["ip"];
                std::string peer_ip;
                P2p p2p;
                p2p.number_to_ip_string(ip, peer_ip);
std::cout << "___02 " << std::endl;
std::cout << "___02 peer_ip " << peer_ip << std::endl;
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
std::cout << "___03 " << std::endl;
        // inform your underlying ones from this block
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
std::cout << "___04 " << std::endl;
        Crowd::Protocol proto;
        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, next_full_hash);
std::cout << "___05 " << std::endl;
        nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "new_final_block";
        message_j["latest_block_nr"] = buf_j["latest_block_nr"];
        message_j["block"] = buf_j["block"];
        message_j["full_hash_coord"] = buf_j["full_hash_coord"];
        message_j["rocksdb"] = buf_j["rocksdb"];
std::cout << "___06 " << std::endl;
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
std::cout << "___07 " << std::endl;
        Crowd::P2pNetwork pn;
        std::string key, val;
        for (auto &[key, val] : parts)
        {
std::cout << "___08 " << std::endl;
            if (key == 1) continue;
            if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
            if (val == full_hash_coord_from_coord) continue;
std::cout << "___09 " << std::endl;
            Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");

            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
            uint32_t peer_ip = value_j["ip"];

            delete rocksy;
            
            std::string message = message_j.dump();

            std::cout << "Preparation for final_new_block: " << peer_ip << std::endl;

            std::string ip_from_peer;
            Crowd::P2p p2p;
            p2p.number_to_ip_string(peer_ip, ip_from_peer);

            // p2p_client() to all chosen ones with intro_peer request
            pn.p2p_client(ip_from_peer, message);
        }
    }
    else
    {
        std::cout << "Final coordinator is not truthful" << std::endl;
    }
}

void P2pNetwork::new_final_block(nlohmann::json buf_j)
{
    // new_final_block
    std::cout << "New_final_block: " << std::endl;

    // Is the coordinator the truthful final coordinator for this block?
    // Compare the hashes from the block of the coordinator with your saved blocks' hashes
    // communicate comparison to other chosen_ones --> needs to be calculated depending on this server's place in the chosen_ones list
    // Then inform your underlying network

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());

    nlohmann::json recv_block_j = buf_j["block"];
    std::string recv_latest_block_nr_s = buf_j["latest_block_nr"];
    std::string full_hash_coord_from_coord = buf_j["full_hash_coord"];
    nlohmann::json chosen_ones = buf_j["chosen_ones"];
    nlohmann::json rocksdb_j = buf_j["rocksdb"];

std::cout << "block_nr: " << recv_latest_block_nr_s << std::endl;
std::cout << "block: " << recv_block_j.dump() << std::endl;

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
    std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be

    std::string recv_block_s = recv_block_j.dump();
    Common::Crypto crypto;
    std::string prev_hash_me = crypto.bech32_encode_sha256(recv_block_s);

    Protocol protocol;
    std::string prev_hash_from_saved_block_at_place_i = protocol.get_block_at(recv_latest_block_nr_s);

    bool comparison = prev_hash_me == prev_hash_from_saved_block_at_place_i;
    std::cout << "New comparison is: " << comparison << std::endl;

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
            Rocksy* rocksy = new Rocksy("usersdbreadonly");
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(c_one));
            delete rocksy;

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

    Crowd::Protocol proto;
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

    Crowd::P2pNetwork pn;
    std::string key, val;
    for (auto &[key, val] : parts)
    {
        if (key == 1) continue;
        if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
        if (val == full_hash_coord_from_coord) continue;
        
        Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");

        // lookup in rocksdb
        nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
        uint32_t peer_ip = value_j["ip"];

        delete rocksy;
        
        std::string message = message_j.dump();

        std::cout << "Preparation for prel_new_block: " << peer_ip << std::endl;

        std::string ip_from_peer;
        Crowd::P2p p2p;
        p2p.number_to_ip_string(peer_ip, ip_from_peer);

        // p2p_client() to all chosen ones with intro_peer request
        pn.p2p_client(ip_from_peer, message);
    }
}

void P2pNetwork::your_full_hash(nlohmann::json buf_j)
{
    // my full hash
    std::string full_hash = buf_j["full_hash"];
    std::string prev_hash = buf_j["prev_hash"];
    std::cout << "New peer's full_hash (server): " << full_hash << std::endl;
    std::cout << "New peer's prev_hash (server): " << prev_hash << std::endl;

    // save full_hash
    FullHash fh;
    fh.save_full_hash_to_file(full_hash);

    // save prev_hash
    PrevHash ph;
    ph.save_my_prev_hash_to_file(prev_hash);
    
    nlohmann::json block_j = buf_j["block"];
    std::string req_latest_block_nr = buf_j["block_nr"];
// std::cout << "block_nr: " << req_latest_block_nr << std::endl;
// std::cout << "block: " << block_j.dump() << std::endl;

    // Update my blocks and rocksdb_
    Protocol proto;
    std::string my_latest_block = proto.get_last_block_nr();
    nlohmann::json m_j;
    m_j["req"] = "update_my_blocks_and_rocksdb";
    m_j["block_nr"] = my_latest_block;
    set_resp_msg_server(m_j.dump());

    // Update my matrices
    nlohmann::json mm_j;
    mm_j["req"] = "update_my_matrices";
    set_resp_msg_server(mm_j.dump());
}

void P2pNetwork::update_my_matrices(nlohmann::json buf_j)
{
    std::cout << "update_my_matrices server" << std::endl;

    nlohmann::json block_matrix_j = buf_j["bm"];
    nlohmann::json intro_msg_s_matrix_j = buf_j["imm"];
    nlohmann::json ip_all_hashes_j = buf_j["iah"];

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
                std::pair<enet_uint32, std::string> myPair = std::make_pair(v3["first"], v3["second"]);
                std::shared_ptr<std::pair<enet_uint32, std::string>> ptr(new std::pair<enet_uint32, std::string> (myPair));
                iah.add_ip_hemail_to_ip_all_hashes_vec(ptr);
            }

            iah.add_ip_all_hashes_vec_to_ip_all_hashes_2d_mat();
        }

        iah.add_ip_all_hashes_2d_mat_to_ip_all_hashes_3d_mat();
    }

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());
}

void P2pNetwork::hash_comparison(nlohmann::json buf_j)
{
    // compare the received hash
    std::cout << "The hash comparison is (server): " <<  buf_j["hash_comp"] << std::endl;
}

void P2pNetwork::update_my_blocks_and_rocksdb(nlohmann::json buf_j)
{
    std::cout << "update_your_blocks_and_rocksdb server" << std::endl;
    // send blocks to peer

    Protocol proto;
    std::string my_latest_block = proto.get_last_block_nr();
    std::string req_latest_block = buf_j["block_nr"];

    nlohmann::json list_of_blocks_j = proto.get_blocks_from(req_latest_block);

    uint64_t my_value;
    std::istringstream iss(my_latest_block);
    iss >> my_value;

    uint64_t req_value;
    std::istringstream isss(req_latest_block);
    isss >> req_value;

    uint64_t next_req_value = req_value + 1;

//std::cout << "next_req_value: " << next_req_value << ", my_value:" << my_value << std::endl;
    for (uint64_t i = next_req_value; i <= my_value; i++)
    {
        nlohmann::json block_j = list_of_blocks_j[i]["block"];
//std::cout << "block_j: " << block_j << std::endl;
//std::cout << "iiiiiiii: " << i << std::endl;
        nlohmann::json msg;
        msg["req"] = "update_your_blocks";
        std::string block_nr_j = list_of_blocks_j[i]["block_nr"];
        msg["block_nr"] = block_nr_j;
//std::cout << "block_nr_j: " << block_nr_j << std::endl;
        msg["block"] = block_j;
        set_resp_msg_client(msg.dump());
    }

    // Update rockdb's:
    nlohmann::json list_of_users_j = nlohmann::json::parse(proto.get_all_users_from(req_latest_block)); // TODO: there are double parse/dumps everywhere
                                                                                                        // maybe even a stack is better ...
    Rocksy* rocksy = new Rocksy("usersdbreadonly");
    for (auto& user : list_of_users_j)
    {
        nlohmann::json msg;
        msg["req"] = "update_your_rocksdb";
        msg["key"] = user;

        std::string u = user;
        std::string value = rocksy->Get(u);
        msg["value"] = value;

        set_resp_msg_server(msg.dump());
    }
    delete rocksy;
}

void P2pNetwork::update_your_matrices_server(nlohmann::json buf_j)
{
    nlohmann::json block_matrix_j = buf_j["bm"];
    nlohmann::json intro_msg_s_matrix_j = buf_j["imm"];
    nlohmann::json ip_all_hashes_j = buf_j["iah"];

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
                std::pair<enet_uint32, std::string> myPair = std::make_pair(v3["first"], v3["second"]);
                std::shared_ptr<std::pair<enet_uint32, std::string>> ptr(new std::pair<enet_uint32, std::string> (myPair));
                iah.add_ip_hemail_to_ip_all_hashes_vec(ptr);
            }

            iah.add_ip_all_hashes_vec_to_ip_all_hashes_2d_mat();
        }

        iah.add_ip_all_hashes_2d_mat_to_ip_all_hashes_3d_mat();
    }
}

void P2pNetwork::intro_online(nlohmann::json buf_j)
{
    std::cout << "intro new peer online: " << buf_j["full_hash"] << std::endl;
    
    nlohmann::json to_verify_j;
    to_verify_j["req"] = buf_j["req"];
    to_verify_j["full_hash"] = buf_j["full_hash"];
    to_verify_j["latest_block_nr"] = buf_j["latest_block_nr"];
    to_verify_j["ip"] = buf_j["ip"];
    to_verify_j["server"] = buf_j["server"];
    to_verify_j["fullnode"] = buf_j["fullnode"];

    Rocksy* rocksy = new Rocksy("usersdb");
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
        std::cout << "verified new online user" << std::endl;

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

        std::string key, val;
        for (auto &[key, val] : parts)
        {
            if (key == 1) continue;
            if (val == full_hash) continue;
            if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
            
            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
            uint32_t peer_ip = value_j["ip"];
            
            std::cout << "Preparation for new_online: " << peer_ip << std::endl;

            std::string ip_from_peer;
            P2p p2p;
            p2p.number_to_ip_string(peer_ip, ip_from_peer);

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
            //std::cout << "list_of_blocks_s: " << list_of_blocks_j.dump() << std::endl;
            uint64_t value;
            std::istringstream iss(my_latest_block);
            iss >> value;

            for (uint64_t i = 0; i <= value; i++)
            {
                nlohmann::json block_j = list_of_blocks_j[i]["block"];
                //std::cout << "block_j: " << block_j << std::endl;
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
        std::cout << "verification new online user not correct" << std::endl;
    }

    delete crypto;

    // Disconect from client
    nlohmann::json msg_j;
    msg_j["req"] = "close_this_conn";
    set_resp_msg_server(msg_j.dump());
}

void P2pNetwork::new_online(nlohmann::json buf_j)
{
    std::cout << "new peer online: " << buf_j["full_hash"] << ", inform your bucket" << std::endl;

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
    
    std::string key, val;
    for (auto &[key, val] : parts)
    {
        if (key == 1) continue;
        if (val == full_hash) continue;
        if (val == my_full_hash || val == "") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
        
        // lookup in rocksdb
        nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
        uint32_t peer_ip = value_j["ip"];
        
        std::string message_s = message_j.dump();

        std::cout << "Preparation for new_online: " << peer_ip << std::endl;

        std::string ip_from_peer;
        P2p p2p;
        p2p.number_to_ip_string(peer_ip, ip_from_peer);

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

void P2pNetwork::set_resp_msg_server(std::string msg)
{
    std::vector<std::string> splitted = split(msg, p2p_message::max_body_length);
    for (int i = 0; i < splitted.size(); i++)
    {
        char s[p2p_message::max_body_length];
        strncpy(s, splitted[i].c_str(), sizeof(s));

        resp_msg_.body_length(std::strlen(s));
        std::memcpy(resp_msg_.body(), s, resp_msg_.body_length());
        i == splitted.size() - 1 ? resp_msg_.encode_header(1) : resp_msg_.encode_header(0); // 1 indicates end of message eom, TODO perhaps a set_eom_flag(true) instead of an int

        // sprintf(buffer_, "%s", (char*) resp_j.dump());
        packet_ = enet_packet_create(resp_msg_.data(), strlen(resp_msg_.data())+1, 0);
        enet_peer_send(event_.peer, 0, packet_);
        enet_host_flush(server_);
    }
}

int P2pNetwork::p2p_server()
{
    int  i;

    if (enet_initialize() != 0)
    {
        printf("Could not initialize enet.");
        return 0;
    }

    address_.host = ENET_HOST_ANY;
    address_.port = PORT;

    server_ = enet_host_create(&address_, 210, 2, 0, 0);

    if (server_ == NULL)
    {
        printf("Could not start server.\n");
        return 0;
    }
    while (1)
    {
        if (get_quit_server_req() == true) break;
        
        while (enet_host_service(server_, &event_, 1000) > 0)
        {
            if (get_quit_server_req() == true) break;

            switch (event_.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    sprintf(read_msg_.data(), "%s", (char*) event_.packet->data);
                    do_read_header_server();



                    // if (event_.peer->data == NULL)
                    // {
                    //     event_.peer->data = malloc(strlen((char*) event_.packet->data)+1);
                    //     strcpy((char*) event_.peer->data, (char*) event_.packet->data);
                    //     sprintf(buffer_, "%s has connected\n", (char*) event_.packet->data);
                    //     packet_ = enet_packet_create(buffer_, strlen(buffer_)+1, 0);
                    //     enet_host_broadcast(server_, 1, packet_);
                    //     enet_host_flush(server_);
                    // }
                    // else
                    // {
                    //     for (int i = 0; i < server_->peerCount; i++)
                    //     {
                    //         if (&server_->peers[i] != event_.peer)
                    //         {
                    //             sprintf(buffer_, "%s: %s", (char*) event_.peer->data, (char*) event_.packet->data);
                    //             packet_ = enet_packet_create(buffer_, strlen(buffer_)+1, 0);
                    //             enet_peer_send(&server_->peers[i], 0, packet_);
                    //             enet_host_flush(server_);
                    //         }
                    //         else
                    //         {

                    //         }
                    //     }
                    // }
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Peer has disconnected." << std::endl;
                    sprintf(buffer_, "%s has disconnected.", (char*) event_.peer->data);
                    packet_ = enet_packet_create(buffer_, strlen(buffer_)+1, 0);
                    enet_host_broadcast(server_, 1, packet_);
                    free(event_.peer->data);
                    event_.peer->data = NULL;
                    break;

                default:
                    printf("Tick tock.\n");
                    break;
            }

        }
    }

    enet_host_destroy(server_);
    enet_deinitialize();

    return 1;
}