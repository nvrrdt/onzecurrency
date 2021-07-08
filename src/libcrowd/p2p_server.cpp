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
        req_conversion["intro_block"] =         7;
        req_conversion["new_block"] =           8;
        req_conversion["your_full_hash"] =      9;
        req_conversion["hash_comparison"] =     10;
        req_conversion["update_my_blocks_and_rocksdb"] = 11;
        req_conversion["intro_online"] =        12;
        req_conversion["new_online"] =          13;

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
            case 7:     intro_block(buf_j);
                        break;
            case 8:     new_block(buf_j);
                        break;
            case 9:     your_full_hash(buf_j);
                        break;
            case 10:    hash_comparison(buf_j);
                        break;
            case 11:    update_my_blocks_and_rocksdb(buf_j);
                        break;
            case 12:    intro_online(buf_j);
                        break;
            case 13:    new_online(buf_j);
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

        PrevHash prev_hash;
        std::string prel_first_prev_hash_req = prev_hash.calculate_hashes_from_last_block_vector().at(0);

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
            
            // update blockchains and rockdb's
            if (req_latest_block < my_latest_block || req_latest_block == "no blockchain present in folder")
            {
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

                Rocksy* rocksy = new Rocksy("usersdb");
                for (auto& user : list_of_users_j) // TODO better make a string with all keys with its values and send that once
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

                // Update your BlockMatrix:
                nlohmann::json msg;
                Poco::BlockMatrix bm;
                nlohmann::json contents_j;
                msg["req"] = "update_your_block_matrix";
                for (int i = 0; i < bm.get_block_matrix().size(); i++)
                {
                    for (int j = 0; j < bm.get_block_matrix().at(i).size(); j++)
                    {
                        contents_j[std::to_string(i)][std::to_string(j)] = *bm.get_block_matrix().at(i).at(j);
                    }
                }
                msg["bm"] = contents_j.dump();

                set_resp_msg_server(msg.dump());

            }
            else if (req_latest_block > my_latest_block)
            {
                // TODO: update your own blockchain
                nlohmann::json msg;
                msg["req"] = "update_my_blocks_and_rocksdb";
                msg["block_nr"] = my_latest_block;
                set_resp_msg_server(msg.dump());
            }
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

        // for loop over latest block_vector and new_co if your not coordinator --> remember already used coordinators for testability




        
//         Crypto* crypto = new Crypto();
//         PrevHash prev_hash;
//         std::string real_prev_hash_req = prev_hash.calculate_hash_from_last_block();

//         std::string hash_of_email_prev_hash_concatenated = hash_of_email + real_prev_hash_req; // TODO should this anonymization not be numbers instead of strings?
//         std::string full_hash_req =  crypto->bech32_encode_sha256(hash_of_email_prev_hash_concatenated);
// std::cout << "______: " << real_prev_hash_req << " , " << email_of_req << " , " << full_hash_req << std::endl;
//         Rocksy* rocksy = new Rocksy("usersdb");
//         if (rocksy->Get(full_hash_req) == "")
//         {
//             delete rocksy;

//             nlohmann::json to_verify_j;
//             to_verify_j["ecdsa_pub_key"] = ecdsa_pub_key_s;
//             to_verify_j["rsa_pub_key"] = rsa_pub_key;
//             to_verify_j["email"] = email_of_req;

//             std::string to_verify_s = to_verify_j.dump();
//             ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
//             crypto->ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
//             std::string signature_bin = crypto->base64_decode(signature);
            
//             if (crypto->ecdsa_verify_message(public_key_ecdsa, to_verify_s, signature_bin))
//             {
//                 // std::cout << "verification1p succeeded: " << std::endl;
//                 // std::cout << "ecdsa_p_key: " << "X" << ecdsa_pub_key_s << "X" << std::endl;
//                 // std::cout << "to_sign_s: " << "X" << to_verify_s << "X" << std::endl;
//                 // std::cout << "base64_signature: " << "X" << signature << "X" << std::endl; 
//                 // std::cout << "signature_bin: " << "X" << signature_bin << "X" << std::endl; 

//                 std::cout << "verified" << std::endl;

//                 // if it's mother peer who has been contacted then lookup chosen_one and communicate that co
//                 // then room_.leave()
//                 // if co then updates ...

//                 Rocksy* rocksy = new Rocksy("usersdb");
//                 std::string co_from_this_server = rocksy->FindChosenOne(full_hash_req);
//                 delete rocksy;
//                 // std::cout << "co_from_this_db: " << co_from_this_db << std::endl;
//                 // std::cout << "co_from_req: " << co_from_req << std::endl;
                
//                 // if (my_full_hash == co_from_this_server) update and recalculate full_hash!!! and create and communicate full_hash
//                 // else room_.deliver ip of co_from_this_server
//                 FullHash fh;
//                 std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be
//                 // std::cout << "My_full_hash already present in file: " << my_full_hash << std::endl;
                
//                 if (my_full_hash == co_from_this_server)
//                 {
//                     std::cout << "my_full_hash: " << my_full_hash << std::endl;

//                     Protocol proto;
//                     std::string my_latest_block = proto.get_last_block_nr();
//                     // std::cout << "My latest block: " << my_latest_block << std::endl;
//                     // std::cout << "Req latest block: " << req_latest_block << std::endl;
                    
//                     // update blockchains and rockdb's
//                     if (req_latest_block < my_latest_block || req_latest_block == "no blockchain present in folder")
//                     {
//                         // TODO: upload blockchain to the requester starting from latest block
//                         // Update blockchain: send latest block to peer
//                         nlohmann::json list_of_blocks_j = proto.get_blocks_from(req_latest_block);
//                         //std::cout << "list_of_blocks_s: " << list_of_blocks_j.dump() << std::endl;
//                         uint64_t value;
//                         std::istringstream iss(my_latest_block);
//                         iss >> value;

//                         for (uint64_t i = 0; i <= value; i++)
//                         {
//                             nlohmann::json block_j = list_of_blocks_j[i]["block"];
//                             //std::cout << "block_j: " << block_j << std::endl;
//                             nlohmann::json msg;
//                             msg["req"] = "update_your_blocks";
//                             std::ostringstream o;
//                             o << i;
//                             msg["block_nr"] = o.str();
//                             msg["block"] = block_j;
//                             set_resp_msg_server(msg.dump());
//                         }

//                         // Update rockdb's:
//                         // How to? Starting from the blocks? Lookup all users in the blocks starting from a block
//                         // , then lookup those user_id's in rocksdb and send
//                         nlohmann::json list_of_users_j = nlohmann::json::parse(proto.get_all_users_from(req_latest_block)); // TODO: there are double parse/dumps everywhere
//                                                                                                                             // maybe even a stack is better ...
//                         Rocksy* rocksy = new Rocksy("usersdb");        // TODO need to handle the online presence of the other users!!!!!
//                         for (auto& user : list_of_users_j) // TODO better make a map of all keys with its values and send that once
//                         {
//                             nlohmann::json msg;
//                             msg["req"] = "update_your_rocksdb";
//                             msg["key"] = user;

//                             std::string u = user;
//                             std::string value = rocksy->Get(u);
//                             msg["value"] = value;

//                             set_resp_msg_server(msg.dump());
//                         }
//                         delete rocksy;
//                     }
//                     else if (req_latest_block > my_latest_block)
//                     {
//                         // TODO: update your own blockchain
//                         nlohmann::json msg;
//                         msg["req"] = "update_my_blocks_and_rocksdb";
//                         msg["block_nr"] = my_latest_block;
//                         set_resp_msg_server(msg.dump());
//                     }


//                     // Disconect from client
//                     nlohmann::json msg_j;
//                     msg_j["req"] = "close_this_conn";
//                     set_resp_msg_server(msg_j.dump());

//                     std::cout << "1 or more totalamountofpeers! " << std::endl;

//                     // communicate intro_peers to chosen_one's with a new_peer req

//                     std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

//                     nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
//                     message_j["req"] = "new_peer";
//                     message_j["email_of_req"] = email_of_req; // new_peers don't need to know this
//                     hash_of_email_prev_hash_concatenated = hash_of_email + real_prev_hash_req; // TODO should this anonymization not be numbers instead of strings?
//                     full_hash_req =  crypto->bech32_encode_sha256(hash_of_email_prev_hash_concatenated);
//                     message_j["full_hash_req"] = full_hash_req; // refreshed full_hash_req
//                     message_j["prev_hash_of_req"] = real_prev_hash_req;
//                     message_j["full_hash_co"] = my_full_hash;
//                     message_j["ecdsa_pub_key"] = ecdsa_pub_key_s;
//                     message_j["rsa_pub_key"] = rsa_pub_key;
//                     message_j["ip"] = event_.peer->address.host;

//                     to_sign_j["ecdsa_pub_key"] = ecdsa_pub_key_s;
//                     to_sign_j["rsa_pub_key"] = rsa_pub_key;
//                     to_sign_j["email"] = email_of_req;
//                     std::string to_sign_s = to_sign_j.dump();
//                     ECDSA<ECP, SHA256>::PrivateKey private_key;
//                     std::string signature;
//                     crypto->ecdsa_load_private_key_from_string(private_key);
//                     if (crypto->ecdsa_sign_message(private_key, to_sign_s, signature))
//                     {
//                         message_j["signature"] = crypto->base64_encode(signature);
//                     }

//                     std::string key, val;
//                     for (int i = 1; i <= parts.size(); i++)
//                     {
//                         // std::cout << "i: " << i << ", val: " << parts[i] << std::endl;
//                         if (i == 1) continue; // ugly hack for a problem in proto.partition_in_buckets()
//                         if (parts[i] == "0" || parts[i] == "") continue; // UGLY hack: "" should be "0"

                        
//                         Rocksy* rocksy = new Rocksy("usersdb");

//                         // lookup in rocksdb
//                         std::string val = parts[i];
//                         nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
//                         enet_uint32 ip = value_j["ip"];
//                         std::string peer_ip;
//                         P2p p2p;
//                         p2p.number_to_ip_string(ip, peer_ip);

//                         std::string req_ip_quad;
//                         p2p.number_to_ip_string(req_ip, req_ip_quad);

//                         delete rocksy;

//                         // if peer ip == this server's ip --> send new_peer to kids
//                         // --> from_to(my_hash, my_hash) if just me then connected_peers + from_to(my_hash, next hash)
//                         // --> if more then t.client to same layer co
//                         // in bucket --> poco --> coord connects to all co --> co connect to other co --> communicate final_hash --> register amount of ok's and nok's

//                         // inform the underlying network
//                         if (req_ip_quad == peer_ip)
//                         {
//                             // inform server's underlying network
//                             std::cout << "Inform my underlying network as co" << std::endl;

//                             std::string next_hash = parts[2];
//                             std::map<int, std::string> parts_underlying = proto.partition_in_buckets(my_full_hash, next_hash);
//                             std::string key2, val2;
//                             Rocksy* rocksy = new Rocksy("usersdb");
//                             for (int i = 1; i <= parts_underlying.size(); i++)
//                             {
//                                 //std::cout << "i2: " << i << " val2: " << parts_underlying[i] << std::endl;
//                                 if (i == 1) continue; // ugly hack for a problem in proto.partition_in_buckets()
//                                 if (parts_underlying[i] == my_full_hash || parts_underlying[i] == "0") continue;
// //std::cout << "parts_underlying: " << parts_underlying[i] << ", my_full_hash: " << my_full_hash << std::endl;
//                                 // lookup in rocksdb
//                                 std::string val2 = parts_underlying[i];
//                                 nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val2));
//                                 enet_uint32 ip = value_j["ip"];
//                                 std::string peer_ip_underlying;
//                                 p2p.number_to_ip_string(ip, peer_ip_underlying);

//                                 std::cout << "Non-connected underlying peers - client: " << peer_ip_underlying << std::endl;
//                                 // message to non-connected peers
//                                 std::string message = message_j.dump();
//                                 p2p_client(peer_ip_underlying, message);
//                             }
//                             delete rocksy;
//                         }
//                         else
//                         {
//                             if (peer_ip == "") continue;

//                             // inform the other peer's in the same layer (as coordinator)
//                             std::cout << "Inform my equal layer as coordinator: " << peer_ip << std::endl;
                            
//                             std::string message = message_j.dump();
//                             p2p_client(peer_ip, message);
//                         }
//                     }

//                     // Update rocksdb
//                     message_j["rocksdb"]["prev_hash"] = real_prev_hash_req;
//                     message_j["rocksdb"]["full_hash"] = full_hash_req;

//                     // wait 20 seconds of > 1 MB to create block, to process the timestamp if you are the first new_peer request
//                     message_j_vec_.add_to_message_j_vec(message_j);

//                     all_hashes_.add_to_all_hashes(message_j["ip"], full_hash_req, real_prev_hash_req); // TODO you have to reset this

//                     if (message_j_vec_.get_message_j_vec().size() > 2048) // 2048x 512 bit hashes
//                     {
//                         // Create block
//                         Poco::PocoCrowd poco;
//                         poco.create_and_send_block ();

//                         for (auto &[key, value] : all_hashes_.get_all_hashes())
//                         {
//                             nlohmann::json msg_j;
//                             msg_j["req"] = "your_full_hash";
//                             std::vector<std::string> vec = *value;
//                             msg_j["full_hash"] = vec[0];
//                             msg_j["prev_hash"] = vec[1];
//                             msg_j["block_nr"] = proto.get_last_block_nr();
//                             std::string msg_s = msg_j.dump();

//                             std::string peer_ip;
//                             P2p p2p;
//                             p2p.number_to_ip_string(key, peer_ip);

//                             p2p_client(peer_ip, msg_s);
//                         }

//                         message_j_vec_.reset_message_j_vec();
//                         all_hashes_.reset_all_hashes();
//                     }
//                     else if (message_j_vec_.get_message_j_vec().size() == 1)
//                     {
//                         // wait 20 secs
//                         // then create block
//                         // if root_hash == me as coordinator ... connect to all co's
//                         // ... see below at new_peer

//                         std::cout << "Get_sleep_and_create_block" << std::endl;

//                         std::thread t(&P2pNetwork::get_sleep_and_create_block_server, this);
//                         t.detach();
//                     }
//                 }
//                 else
//                 {
//                     // There's another chosen_one, reply with the correct chosen_one
//                     std::cout << "Chosen_one is someone else!" << std::endl;

//                     // room_.deliver() co_from_this_server with new_co request
//                     nlohmann::json message_j;
//                     message_j["req"] = "new_co";
// std::cout << "co from this server:______ " << co_from_this_server << std::endl;

//                     Rocksy* rocksy = new Rocksy("usersdb");
// std::cout << "usersdb size:______ " << rocksy->TotalAmountOfPeers() << std::endl;
//                     nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(co_from_this_server));
//                     enet_uint32 peer_ip = value_j["ip"];
//                     delete rocksy;

//                     message_j["ip_co"] = peer_ip;
//                     set_resp_msg_server(message_j.dump());
//                 }
//             }
//             else
//             {
//                 std::cout << "failed verification" << std::endl;

//                 // std::cout << "verification2p: " << std::endl;
//                 // std::cout << "ecdsa_p_key: " << "X" << ecdsa_pub_key_s << "X" << std::endl;
//                 // std::cout << "to_sign_s: " << "X" << to_verify_s << "X" << std::endl;
//                 // std::cout << "signature: " << "X" << signature << "X" << std::endl;
//             }
//         }
//         else
//         {
//             delete rocksy;

//             // user exists already in rocksdb
//             // respond with user_exists

//             std::cout << "!rocksy->Get(full_hash_req) == \"\" " << std::endl;
//         }

//         delete crypto;
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
    std::cout << "new_peer: " << std::endl;
    // should read the timestamp of the first new_peer request received
    
    // wait 20 seconds of > 1 MB to create block, to process the timestamp if you are the first new_peer request
    message_j_vec_.add_to_message_j_vec(buf_j);

    if (message_j_vec_.get_message_j_vec().size() > 2048) // 2048x 512 bit hashes
    {
        // Create block
        Poco::PocoCrowd poco;
        poco.create_and_send_block();

        message_j_vec_.reset_message_j_vec();
        all_hashes_.reset_all_hashes();
    }
    else if (message_j_vec_.get_message_j_vec().size() == 1)
    {
        // wait 20 secs
        // then create block --> don't forget the counter in the search for a coordinator
        // if root_hash == me as coordinator ... connect to all co's
        std::thread t(&P2pNetwork::get_sleep_and_create_block_server, this);
        t.detach();
    }

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn_without_server";
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

void P2pNetwork::intro_block(nlohmann::json buf_j)
{
    // intro_block
    std::cout << "Intro_block: " << std::endl;

    // Compare the hashes from the block of the coordinator with this block
    // p2p_client to other chosen_ones --> needs to be calculated depending on this server's place in the chosen_ones list
    // Communicate hash to all
    // Then inform your underlying network
    // Put block in waiting list until it's the only block in the chain --> future implementation with coin

    // Is the coordinator the truthful real coordinator for this block

    nlohmann::json recv_block_j = buf_j["block"];
    Poco::BlockMatrix *bm = new Poco::BlockMatrix();
    if (bm->get_block_matrix().empty())
    {
        std::cout << "Received block is in block_vector; 1" << std::endl;
        bm->add_received_block_to_received_block_vector(recv_block_j);
    }
    else
    {
        for (uint16_t i = 0; i < bm->get_block_matrix().back().size(); i++)
        {
            if (*bm->get_block_matrix().back().at(i) == recv_block_j)
            {
                std::cout << "Received block is in block_vector; 2" << std::endl;
                bm->add_received_block_to_received_block_vector(recv_block_j);
                break;
            }
            
            if (i == bm->get_block_matrix().back().size() - 1)
            {
                // Don't accept this block
                std::cout << "Received block not in block_vector" << std::endl;
                return;
            }
        }
    }

    delete bm;

    std::string full_hash_coord_from_coord = buf_j["full_hash_coord"];

    nlohmann::json starttime_coord = buf_j["block"]["starttime"];

    Common::Crypto crypto;
    Poco::PocoCrowd poco;
    
    nlohmann::json block_j_me = poco.get_block_j();
    block_j_me["starttime"] = starttime_coord;
    std::string block_s_me = block_j_me.dump();
    std::string prev_hash_me = crypto.bech32_encode_sha256(block_s_me);

    Rocksy* rocksy = new Rocksy("usersdb");
    std::string full_hash_coord_from_me = rocksy->FindChosenOne(prev_hash_me);
    if (full_hash_coord_from_me == buf_j["full_hash_req"])
    {
        full_hash_coord_from_me = rocksy->FindNextPeer(full_hash_coord_from_me);
    }
    delete rocksy;

    if (full_hash_coord_from_coord == full_hash_coord_from_me)
    {
        std::cout << "Coordinator is truthful" << std::endl;

        std::string prev_hash_coordinator = buf_j["prev_hash"];
        std::string prev_hash_in_block = buf_j["block"]["prev_hash"];

        if (prev_hash_coordinator == prev_hash_me && prev_hash_coordinator == prev_hash_in_block)
        {
            std::cout << "Successful comparison of prev_hashes, now sharing hashes" << std::endl;

            // // Save block
            // merkle_tree mt;
            // std::string latest_block_nr = buf_j["latest_block_nr"];
            // mt.save_block_to_file(block_j_me, latest_block_nr);

            // // Put in rocksdb
            // for (auto &[key, value] : buf_j["rocksdb"].items())
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
            std::cout << "Unsuccessful comparison of prev_hashes" << std::endl;
        }

        // Inform coordinator of succesfullness of hash comparison
        nlohmann::json m_j;
        m_j["req"] = "hash_comparison";
        m_j["hash_comp"] = prev_hash_me == prev_hash_coordinator;
        std::string msg_s = m_j.dump();

        set_resp_msg_server(msg_s);

        // p2p_client() to all calculated other chosen_ones
        // this is in fact the start of the consensus algorithm where a probability is calculated
        // you don't need full consensus in order to create a succesful block
        // but full consensus improves your chances of course greatly
        nlohmann::json chosen_ones = buf_j["chosen_ones"];

        FullHash fh;
        std::string my_full_hash = fh.get_full_hash_from_file(); // TODO this is a file lookup and thus takes time --> static var should be
        // std::cout << "My_full_hash already present in file: " << my_full_hash << std::endl;

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
                Rocksy* rocksy = new Rocksy("usersdb");
                nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(c_one));
                delete rocksy;

                enet_uint32 ip = value_j["ip"];
                std::string peer_ip;
                P2p p2p;
                p2p.number_to_ip_string(ip, peer_ip);

                nlohmann::json msg_j;
                msg_j["req"] = "hash_comparison";
                msg_j["hash_comp"] = prev_hash_me == prev_hash_coordinator;
                std::string msg_s = msg_j.dump();

                p2p_client(peer_ip, msg_s);
            }
            else if (i == j)
            {
                continue;
            }
        }
    }
    else
    {
        std::cout << "Coordinator is not truthful" << std::endl;
    }

    // Disconect from client
    nlohmann::json m_j;
    m_j["req"] = "close_this_conn";
    set_resp_msg_server(m_j.dump());
}

void P2pNetwork::new_block(nlohmann::json buf_j)
{
    // new_block --> TODO block and rocksdb should be saved
    std::cout << "New_block: " << std::endl;
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
std::cout << "block_nr: " << req_latest_block_nr << std::endl;
std::cout << "block: " << block_j.dump() << std::endl;
    // TODO: add most probable blocks to the correct block vector in the block matrix
    // Let's say 25 of latest vector
    // merkle_tree mt;
    // mt.save_block_to_file(block_j,req_latest_block_nr);

    // Put in rocksdb
    for (auto &[key, value] : buf_j["rocksdb"].items())
    {
        std::string key_s = value["full_hash"];
        std::string value_s = value.dump();

        Rocksy* rocksy = new Rocksy("usersdb");
        rocksy->Put(key_s, value_s);
        delete rocksy;
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
    Rocksy* rocksy = new Rocksy("usersdb");
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
            if (val == my_full_hash || val == "" || val == "0") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
            
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
        if (val == my_full_hash || val == "" || val == "0") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other
        
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


/**
 * synchronisation is necessary
 * 
 * 1st peer creation --> 2nd peer creation --> reward tx to 1st peer --> 3rd peer creation --> reward txs to first 2 peers
 * 
 * poco introduction for crowd first ...
 * then trying to make poco work for coin ...
 * 
 * calculating new blocks should be ceased and start over with a new vector after the block creation delay (here 10 seconds)
 */


void P2pNetwork::get_sleep_and_create_block_server()
{
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "message_j_vec.size() in Poco: " << message_j_vec_.get_message_j_vec().size() << std::endl;
    std::cout << "transactions.size() in Coin: " << tx_.get_transactions().size() << std::endl;

    // chosen ones are being informed here
    bm_->add_received_block_vector_to_received_block_matrix();
    std::thread t(&Poco::PocoCrowd::create_and_send_block, pococr_);

    // and here too, but for coin then
    bmc_->add_received_block_vector_to_received_block_matrix();
    // pococo_.create_and_send_block_c();     // preliminary commented out
    
    t.join();

    std::cout << "Blocks created server!!" << std::endl;
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