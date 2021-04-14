#include "p2p_network.hpp"

using namespace Crowd;

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
        buf_ += str_read_msg.substr(0, read_msg_.get_body_length());
        
    } else {
        // process json message
        std::string str_read_msg(read_msg_.body());
        buf_ += str_read_msg.substr(0, read_msg_.get_body_length());

        nlohmann::json buf_j = nlohmann::json::parse(buf_);
        if (buf_j["req"] == "register")
        {
            nlohmann::json resp_j;
            resp_j["register"] = "ack";

            set_resp_msg_server(resp_j.dump());
            
            std::cout << "Ack for registering client is confirmed" << std::endl;
        }
        else if (buf_j["req"] == "connect")
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
        else if (buf_j["req"] == "intro_peer")
        {
            // process buf_j["hash_of_req"] to find ip of the peer who should update you
            std::string co_from_req = buf_j["full_hash_co"];
            std::string email_of_req = buf_j["email_of_req"];
            // std::string prev_hash_req = buf_j["prev_hash_of_req"];
            std::string ecdsa_pub_key_s = buf_j["ecdsa_pub_key"];
            std::string rsa_pub_key = buf_j["rsa_pub_key"];
            //std::cout << "1p: " << buf_j.dump() << std::endl;
            std::string signature = buf_j["signature"];
            //std::cout << "2p: " << std::endl;
            std::string req_latest_block = buf_j["latest_block"];
            enet_uint32 req_ip = buf_j["ip"];

            Crypto* crypto = new Crypto();
            PrevHash prev_hash;
            std::string real_prev_hash_req = prev_hash.get_prev_hash_from_the_last_block();
            std::string email_prev_hash_concatenated = email_of_req + real_prev_hash_req;
            std::string full_hash_req =  crypto->bech32_encode_sha256(email_prev_hash_concatenated);

            Rocksy* rocksy = new Rocksy();
            if (rocksy->Get(full_hash_req) == "")
            {
                delete rocksy;

                nlohmann::json to_verify_j;
                to_verify_j["ecdsa_pub_key"] = ecdsa_pub_key_s;
                to_verify_j["rsa_pub_key"] = rsa_pub_key;
                to_verify_j["email"] = email_of_req;

                std::string to_verify_s = to_verify_j.dump();
                ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
                crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
                std::string signature_bin = crypto->base64_decode(signature);
                
                if (crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
                {
                    // std::cout << "verification1p succeeded: " << std::endl;
                    // std::cout << "ecdsa_p_key: " << "X" << ecdsa_pub_key_s << "X" << std::endl;
                    // std::cout << "to_sign_s: " << "X" << to_verify_s << "X" << std::endl;
                    // std::cout << "base64_signature: " << "X" << signature << "X" << std::endl; 
                    // std::cout << "signature_bin: " << "X" << signature_bin << "X" << std::endl; 

                    std::cout << "verified" << std::endl;

                    // if it's mother peer who has been contacted then lookup chosen_one and communicate that co
                    // then room_.leave()
                    // if co then updates ...

                    Rocksy* rocksy = new Rocksy();
                    std::string co_from_this_server = rocksy->FindChosenOne(full_hash_req);
                    delete rocksy;
                    // std::cout << "co_from_this_db: " << co_from_this_db << std::endl;
                    // std::cout << "co_from_req: " << co_from_req << std::endl;
                    
                    // if (my_full_hash == co_from_this_server) update and recalculate full_hash!!! and create and communicate full_hash
                    // else room_.deliver ip of co_from_this_server
                    Auth a;
                    std::string my_full_hash = a.get_my_full_hash();
                    if (my_full_hash == co_from_this_server)
                    {
                        std::cout << "my_full_hash: " << my_full_hash << std::endl;

                        Protocol proto;
                        std::string my_latest_block = proto.get_last_block_nr();
                        // std::cout << "My latest block: " << my_latest_block << std::endl;
                        // std::cout << "Req latest block: " << req_latest_block << std::endl;
                        
                        // update blockchains and rockdb's
                        if (req_latest_block < my_latest_block || req_latest_block == "no blockchain present in folder")
                        {
                            // TODO: upload blockchain to the requester starting from latest block
                            // Update blockchain: send latest block to peer
                            nlohmann::json list_of_blocks_j = nlohmann::json::parse(proto.get_blocks_from(req_latest_block));
                            //std::cout << "list_of_blocks_s: " << list_of_blocks_j.dump() << std::endl;
                            uint64_t value;
                            std::istringstream iss(my_latest_block);
                            iss >> value;

                            for (uint64_t i = 0; i < value; i++)
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
                            Rocksy* rocksy = new Rocksy();        // TODO need to handle the online presence of the other users!!!!!
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
                            delete rocksy;
                        }
                        else if (req_latest_block > my_latest_block)
                        {
                            // TODO: update your own blockchain
                            nlohmann::json msg;
                            msg["req"] = "update_my_blocks_and_rocksdb";
                            msg["block_nr"] = my_latest_block;
                            set_resp_msg_server(msg.dump());
                        }


                        // Disconect from client
                        nlohmann::json msg_j;
                        msg_j["req"] = "close_this_conn";
                        set_resp_msg_server(msg_j.dump());

                        std::cout << "1 or more totalamountofpeers! " << std::endl;

                        // communicate intro_peers to chosen_one's with a new_peer req

                        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

                        nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
                        message_j["req"] = "new_peer";
                        message_j["email_of_req"] = email_of_req; // new_peers don't need to know this
                        email_prev_hash_concatenated = email_of_req + real_prev_hash_req;
                        full_hash_req =  crypto->bech32_encode_sha256(email_prev_hash_concatenated);
                        message_j["full_hash_req"] = full_hash_req; // refreshed full_hash_req
                        message_j["prev_hash_of_req"] = real_prev_hash_req;
                        message_j["full_hash_co"] = my_full_hash;
                        message_j["ecdsa_pub_key"] = ecdsa_pub_key_s;
                        message_j["rsa_pub_key"] = rsa_pub_key;
                        message_j["ip"] = event_.peer->address.host;

                        to_sign_j["ecdsa_pub_key"] = ecdsa_pub_key_s;
                        to_sign_j["rsa_pub_key"] = rsa_pub_key;
                        to_sign_j["email"] = email_of_req;
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
                            // std::cout << "i: " << i << ", val: " << parts[i] << std::endl;
                            if (parts[i] == "0" || parts[i] == "") continue; // UGLY hack: "" should be "0"

                            
                            Rocksy* rocksy = new Rocksy();

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
                            // in bucket --> createblock --> coord connects to all co --> co connect to other co --> communicate final_hash --> register amount of ok's and nok's

                            // inform the underlying network
                            if (req_ip_quad == peer_ip)
                            {
                                // inform server's underlying network
                                std::cout << "Inform my underlying network as co" << std::endl;

                                std::string next_hash = parts[2];
                                std::map<int, std::string> parts_underlying = proto.partition_in_buckets(my_full_hash, next_hash);
                                std::string key2, val2;
                                Rocksy* rocksy = new Rocksy();
                                for (int i = 1; i <= parts_underlying.size(); i++)
                                {
                                    //std::cout << "i2: " << i << " val2: " << parts_underlying[i] << std::endl;
                                    if (parts_underlying[i] == my_full_hash || parts_underlying[i] == "0") continue;

                                    // lookup in rocksdb
                                    std::string val2 = parts_underlying[i];
                                    nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val2));
                                    enet_uint32 ip = value_j["ip"];
                                    std::string peer_ip_underlying;
                                    p2p.number_to_ip_string(ip, peer_ip_underlying);

                                    std::cout << "Non-connected underlying peers - client: " << peer_ip << std::endl;
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
                                std::cout << "Inform my equal layer as coordinator: " << peer_ip << std::endl;
                                
                                std::string message = message_j.dump();
                                p2p_client(peer_ip, message);
                            }
                        }

                        // Update rocksdb
                        message_j["rocksdb"]["prev_hash"] = real_prev_hash_req;
                        message_j["rocksdb"]["full_hash"] = full_hash_req;

                        // wait 20 seconds of > 1 MB to create block, to process the timestamp if you are the first new_peer request
                        message_j_vec_.add_to_message_j_vec(message_j);

                        all_full_hashes_.add_to_all_full_hashes(event_.peer->address.host, full_hash_req); // TODO you have to reset this

                        if (message_j_vec_.get_message_j_vec().size() > 2048) // 2048x 512 bit hashes
                        {
                            // Create block
                            std::vector<nlohmann::json> m_j_v = message_j_vec_.get_message_j_vec();
                            std::map<enet_uint32, std::string> a_f_h = all_full_hashes_.get_all_full_hashes();
                            CreateBlock cb(m_j_v, a_f_h);
                            nlohmann::json block_j = cb.get_block_j();

                            for (auto &[key, value] : a_f_h)
                            {
                                nlohmann::json msg_j;
                                msg_j["req"] = "your_full_hash";
                                msg_j["full_hash"] = value;
                                msg_j["block"] = block_j;
                                msg_j["hash_of_block"] = cb.get_hash_of_new_block();
                                std::string msg_s = msg_j.dump();

                                std::string peer_ip;
                                P2p p2p;
                                p2p.number_to_ip_string(key, peer_ip);

                                p2p_client(peer_ip, msg_s);
                            }

                            message_j_vec_.reset_message_j_vec();
                            all_full_hashes_.reset_all_full_hashes();
                        }
                        else if (message_j_vec_.get_message_j_vec().size() == 1)
                        {
                            // wait 20 secs
                            // then create block
                            // if root_hash == me as coordinator ... connect to all co's
                            // ... see below at new_peer

                            std::cout << "Get_sleep_and_create_block" << std::endl;

                            std::thread t(&P2pNetwork::get_sleep_and_create_block_server, this);
                            t.detach();
                        }
                    }
                    else
                    {
                        // There's another chosen_one, reply with the correct chosen_one
                        std::cout << "Chosen_one is someone else!" << std::endl;

                        // room_.deliver() co_from_this_server with new_co request
                        nlohmann::json message_j;
                        message_j["req"] = "new_co";

                        Rocksy* rocksy = new Rocksy();
                        nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(co_from_this_server));
                        enet_uint32 peer_ip = value_j["ip"];
                        delete rocksy;

                        message_j["ip_co"] = peer_ip;
                        set_resp_msg_server(message_j.dump());
                    }
                }
                else
                {
                    std::cout << "failed verification" << std::endl;

                    // std::cout << "verification2p: " << std::endl;
                    // std::cout << "ecdsa_p_key: " << "X" << ecdsa_pub_key_s << "X" << std::endl;
                    // std::cout << "to_sign_s: " << "X" << to_verify_s << "X" << std::endl;
                    // std::cout << "signature: " << "X" << signature << "X" << std::endl;
                }
            }
            else
            {
                delete rocksy;

                // user exists already in rocksdb
                // respond with user_exists

                std::cout << "!rocksy->Get(full_hash_req) == \"\" " << std::endl;
            }

            delete crypto;

            
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

            //     CreateBlock cb(email_of_peer, hash_of_peer); // moet bij new_peer
            //     std::cout << "Is this reached? " << hash_of_peer << std::endl;
            //     // if cb ok: update blockchain and update rocksdb will be received through the chosen one's
            // }
            // else
            // {
            //     // If there are more peers in the ip_list ...
            // }
        }
        else if (buf_j["req"] == "new_peer")
        {
            std::cout << "new_peer: " << std::endl;
            // should read the timestamp of the first new_peer request received
            
            // wait 20 seconds of > 1 MB to create block, to process the timestamp if you are the first new_peer request
            message_j_vec_.add_to_message_j_vec(buf_j);

            std::string full_hash_req = buf_j["full_hash_req"];
            all_full_hashes_.add_to_all_full_hashes(buf_j["ip"], full_hash_req);
            
            if (message_j_vec_.get_message_j_vec().size() > 2048) // 2048x 512 bit hashes
            {
                // Create block
                std::vector<nlohmann::json> m_j_v = message_j_vec_.get_message_j_vec();
                std::map<enet_uint32, std::string> a_f_h = all_full_hashes_.get_all_full_hashes();
                CreateBlock cb(m_j_v, a_f_h);

                message_j_vec_.reset_message_j_vec();
                all_full_hashes_.reset_all_full_hashes();
            }
            else if (message_j_vec_.get_message_j_vec().size() == 1)
            {
                // wait 20 secs
                // then create block --> don't forget the counter in the search for a coordinator
                // if root_hash == me as coordinator ... connect to all co's
                std::thread t(&P2pNetwork::get_sleep_and_create_block_server, this);
                t.detach();
            }
        }
        else if (buf_j["req"] == "update_your_blocks")
        {
            std::cout << "update_your_blocks server" << std::endl;
            // save blocks to blockchain folder

            nlohmann::json block_j = buf_j["block"].get<nlohmann::json>();
            std::string block_nr = buf_j["block_nr"];
            if (block_nr == "no blockchain present in folder") block_nr = "0";
            // std::cout << "block_s: " << buf_j["block"] << std::endl;
            // std::cout << "block_nr: " << block_nr << std::endl;

            merkle_tree mt;
            mt.save_block_to_file(block_j, block_nr);
        }
        else if (buf_j["req"] == "update_your_rocksdb")
        {
            std::cout << "update_your_rocksdb server" << std::endl;

            std::string key_s = buf_j["key"];
            std::string value_s = buf_j["value"];

            Rocksy* rocksy = new Rocksy();
            rocksy->Put(key_s, value_s);
            delete rocksy;
        }
        else if (buf_j["req"] == "intro_block")
        {
            // intro_block
            std::cout << "Intro_block: " << std::endl;

            // Compare the block from the coordinator with this block
            // Hash the block if correct
            // tcp.client to other chosen_ones --> needs to be calculated depending on this server's place in the chosen_ones list
            // Communicate hash to all
            // Then inform your underlying network
            // Put block in waiting list until it's the only block in the chain --> that's a nice idea, how much disk space does it take?

            ssize_t block_size_coordinator = buf_j["block"]["entry"].size();
            std::string prev_hash_coordinator = buf_j["prev_hash"];

            std::vector<nlohmann::json> message_j_vec_size_as_coordinator;
            std::vector<nlohmann::json> m_j_v = message_j_vec_.get_message_j_vec();

            for(int i = 0; i < block_size_coordinator; i++)
            {
                message_j_vec_size_as_coordinator.push_back(m_j_v[i]);
            }

            merkle_tree mt;

            nlohmann::json m_j, entry_tx_j, entry_transactions_j, exit_tx_j, exit_transactions_j;
            nlohmann::json to_block_j;
            std::shared_ptr<std::stack<std::string>> s_shptr = make_shared<std::stack<std::string>>();

            for (int i = 0; i < message_j_vec_size_as_coordinator.size(); i++)
            {
                m_j = message_j_vec_size_as_coordinator[i];

                std::string full_hash_req = m_j["full_hash_req"];

                to_block_j["full_hash"] = full_hash_req;
                to_block_j["ecdsa_pub_key"] = m_j["ecdsa_pub_key"];
                to_block_j["rsa_pub_key"] = m_j["rsa_pub_key"];
                s_shptr->push(to_block_j.dump());

                entry_tx_j["full_hash"] = to_block_j["full_hash"];
                entry_tx_j["ecdsa_pub_key"] = to_block_j["ecdsa_pub_key"];
                entry_tx_j["rsa_pub_key"] = to_block_j["rsa_pub_key"];
                entry_transactions_j.push_back(entry_tx_j);
                exit_tx_j["full_hash"] = "";
                exit_transactions_j.push_back(exit_tx_j);
            }

            s_shptr = mt.calculate_root_hash(s_shptr);
            std::string datetime = mt.time_now();
            std::string root_hash_data = s_shptr->top();
            nlohmann::json block_j = mt.create_block(datetime, root_hash_data, entry_transactions_j, exit_transactions_j);
            std::string block_s = block_j.dump();
            Crypto* crypto = new Crypto;
            std::string prev_hash_chosen_one = crypto->bech32_encode_sha256(block_s);
            delete crypto;

            if (prev_hash_coordinator == prev_hash_chosen_one)
            {
                std::cout << "Successful comparison of prev_hashes" << std::endl;

                // Is the coordinator the truthful real coordinator for this block

                std::string full_hash_coord_from_coord = buf_j["full_hash_coord"];
                Rocksy* rocksy = new Rocksy;
                std::string full_hash_coord_from_this_server = rocksy->FindChosenOne(prev_hash_coordinator);
                delete rocksy;
                if (full_hash_coord_from_coord == full_hash_coord_from_this_server)
                {
                    std::cout << "Successful comparison of coordinator full_hashes" << std::endl;

                    // then tcp.client() to all calculated other chosen_ones
                    // this is in fact the start of the consensus algorithm
                    // you don't need full consensus in order to create a succesful block
                    // but full consensus improves your chances of course greatly
                    nlohmann::json chosen_ones = buf_j["chosen_ones"];
                    Auth a;
                    std::string my_full_hash = a.get_my_full_hash();

                    int j;

                    for (int i = 0; i < chosen_ones.size(); i++)
                    {
                        if (chosen_ones[i] == my_full_hash)
                        {
                            j = i;
                        }
                    }

                    for (int i = 0; i < chosen_ones.size(); i++)
                    {
                        if (i > j)
                        {
                            std::string c_one = chosen_ones[i];
                            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(c_one));

                            enet_uint32 ip = value_j["ip"];
                            std::string peer_ip;
                            P2p p2p;
                            p2p.number_to_ip_string(ip, peer_ip);
                            
                            nlohmann::json msg_j;
                            msg_j["req"] = "hash_comparison";
                            msg_j["hash"] = prev_hash_chosen_one;
                            std::string msg_s = msg_j.dump();

                            p2p_client(peer_ip, msg_s);
                        }
                    }
                }
                else
                {
                    std::cout << "Unsuccessful comparison of coordinator full_hashes" << std::endl;
                }
            }
            else
            {
                std::cout << "Unsuccessful comparison of prev_hashes" << std::endl;
            }
        }
        else if (buf_j["req"] == "new_block")
        {
            // new_block
            std::cout << "New_block: " << std::endl;
        }
        else if (buf_j["req"] == "your_full_hash")
        {
            // my full hash
            std::string full_hash = buf_j["full_hash"];
            std::cout << "New peer's full_hash (server): " << full_hash << std::endl;

            // save full_hash
            P2p p2p;
            p2p.save_full_hash_to_file(full_hash);

            // Save buf_j["block"] to file
            nlohmann::json block_j = buf_j["block"];
            
            std::string req_latest_block = buf_j["block_nr"];

            Protocol proto;
            std::string my_latest_block = proto.get_last_block_nr();

            if (req_latest_block > my_latest_block)
            {
                // TODO: update your own blockchain
                nlohmann::json msg;
                msg["req"] = "update_my_blocks_and_rocksdb";
                msg["block_nr"] = my_latest_block;
                set_resp_msg_server(msg.dump());
            }

            // Disconect from client
            nlohmann::json m_j;
            m_j["req"] = "close_this_conn";
            set_resp_msg_server(m_j.dump());
        }
        else if (buf_j["req"] = "hash_comparison")
        {
            // compare the received hash
            std::cout << "The hash to compare is: " << buf_j["hash"] << std::endl;
        }
        else if (buf_j["req"] == "update_my_blocks_and_rocksdb")
        {
            std::cout << "update_your_blocks_and_rocksdb server" << std::endl;
            // send blocks to peer

            Protocol proto;
            std::string my_latest_block = proto.get_last_block_nr();
            std::string req_latest_block = buf_j["block_nr"];

            nlohmann::json list_of_blocks_j = nlohmann::json::parse(proto.get_blocks_from(req_latest_block));

            uint64_t value;
            std::istringstream iss(my_latest_block);
            iss >> value;

            for (uint64_t i = 0; i < value; i++)
            {
                nlohmann::json block_j = list_of_blocks_j[i]["block"];
                // std::cout << "block_j: " << block_j << std::endl;
                nlohmann::json msg;
                msg["req"] = "update_your_blocks";
                std::ostringstream o;
                o << i;
                msg["block_nr"] = o.str();
                msg["block"] = block_j;
                set_resp_msg_server(msg.dump());
            }

            // Update rockdb's:
            nlohmann::json list_of_users_j = nlohmann::json::parse(proto.get_all_users_from(req_latest_block)); // TODO: there are double parse/dumps everywhere
                                                                                                                // maybe even a stack is better ...
            Rocksy* rocksy = new Rocksy();
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

        buf_ = "";
    }
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

void P2pNetwork::get_sleep_and_create_block_server()
{
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "message_j_vec.size() in CreateBlock: " << message_j_vec_.get_message_j_vec().size() << std::endl;

    std::vector<nlohmann::json> m_j_v = message_j_vec_.get_message_j_vec();
    std::map<enet_uint32, std::string> a_f_h = all_full_hashes_.get_all_full_hashes();
    CreateBlock cb(m_j_v, a_f_h); // chosen ones are being informed here
    nlohmann::json block_j = cb.get_block_j();

    for (auto &[key, value] : a_f_h)
    {
        nlohmann::json msg_j;
        msg_j["req"] = "your_full_hash";
        msg_j["full_hash"] = value;
        msg_j["block"] = block_j;
        Protocol proto;
        msg_j["block_nr"] = proto.get_last_block_nr();
        msg_j["hash_of_block"] = cb.get_hash_of_new_block();
        std::string msg_s = msg_j.dump();

        std::string peer_ip;
        P2p p2p;
        p2p.number_to_ip_string(key, peer_ip);
        
        std::cout << "key: " << key << " ip: " << peer_ip << ", value: " << value << std::endl;
        p2p_client(peer_ip, msg_s);
    }

    message_j_vec_.reset_message_j_vec();
    all_full_hashes_.reset_all_full_hashes();

    std::cout << "Block created server!!" << std::endl;
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
        while (enet_host_service(server_, &event_, 1000) > 0)
        {
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
}