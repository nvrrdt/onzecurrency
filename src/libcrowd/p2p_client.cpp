#include "p2p_network.hpp"
#include "p2p_network_c.hpp"

using namespace Crowd;

std::string P2pNetwork::closed_client_ = "";
uint32_t P2pNetwork::ip_new_co_ = 0;

void P2pNetwork::do_read_header_client()
{
    if (read_msg_.decode_header() == true)
    {
        do_read_body_client();
    }
}

void P2pNetwork::do_read_body_client()
{
    handle_read_client();
}

void P2pNetwork::handle_read_client()
{
    if ( !read_msg_.get_eom_flag())
    {
        std::string str_read_msg(read_msg_.body());
        buf_client_ += str_read_msg;
    }
    else
    {
        // process json message
        std::string str_read_msg(read_msg_.body());
        buf_client_ += str_read_msg.substr(0, read_msg_.get_body_length());
        nlohmann::json buf_j = nlohmann::json::parse(buf_client_);

        std::string req = buf_j["req"];
        std::map<std::string, int> req_conversion;
        req_conversion["register"] =            1;
        req_conversion["connect"] =             2;
        req_conversion["connect_true"] =        3;
        req_conversion["update_your_blocks"] =  4;
        req_conversion["update_your_rocksdb"] = 5;
        req_conversion["update_your_matrices"] =            6;
        req_conversion["update_my_blocks_and_rocksdb"] =    7;
        req_conversion["update_my_matrices"] =  8;
        req_conversion["new_peer"] =            9;
        req_conversion["new_co"] =              10;
        req_conversion["your_full_hash"] =      11;
        req_conversion["hash_comparison"] =     12;
        req_conversion["close_this_conn"] =     13;
        req_conversion["close_this_conn_and_create"] =      14;

        switch (req_conversion[req])
        {
            case 1:     register_for_nat_traversal_client(buf_j);
                        break;
            case 2:     connect_to_nat_client(buf_j);
                        break;
            case 3:     connect_true_client(buf_j);
                        break;
            case 4:     update_your_blocks_client(buf_j);
                        break;
            case 5:     update_your_rocksdb_client(buf_j);
                        break;
            case 6:     update_your_matrices_client(buf_j);
                        break;
            case 7:     update_my_blocks_and_rocksdb_client(buf_j);
                        break;
            case 8:     update_my_matrices_client(buf_j);
                        break;
            case 9:     new_peer_client(buf_j);
                        break;
            case 10:    new_co_client(buf_j);
                        break;
            case 11:    your_full_hash_client(buf_j);
                        break;
            case 12:    hash_comparison_client(buf_j);
                        break;
            case 13:    close_this_conn_client(buf_j);
                        break;
            case 14:    close_this_conn_and_create_client(buf_j);
                        break;
            default:    Coin::P2pNetworkC pnc;
                        pnc.handle_read_client_c(buf_j);
                        break;
        }

        buf_client_ = "";
    }
}

void P2pNetwork::register_for_nat_traversal_client(nlohmann::json buf_j)
{
    if (buf_j["register"] == "ack") // something wrong here when implementing the switch
    {
        // TODO: what if there was no response from the server?

        std::cout << "Ack for registering this client to a server" << std::endl;
    }
}

void P2pNetwork::connect_to_nat_client(nlohmann::json buf_j)
{
    if (buf_j["connect"] == "ok")
    {
        std::cout << "connect = ok" << std::endl;
        nlohmann::json message_j;
        message_j["connect"] = "true";

        // Establishing NAT Traversal
        // TODO: needs to be tested that there is really a connection between the two peers
        if (buf_j["id_from"] == "nvrrdt_from") // TODO: change nvrrdt to my_id/my_hash/my_ip
        {
            std::cout << "message send to id_to from id_from" << std::endl;
            std::string peer_ip = buf_j["ip_to"];
            std::string message = message_j.dump();
            std::string pub_key = "pub_key";
            p2p_client(peer_ip, message);
            p2p_server();
        }
        else
        {
            std::cout << "message send to id_from from id_to" << std::endl;
            std::string peer_ip = buf_j["ip_from"];
            std::string message = message_j.dump();
            std::string pub_key = "pub_key";
            p2p_client(peer_ip, message);
            p2p_server();
        }
    }
}

void P2pNetwork::connect_true_client(nlohmann::json buf_j)
{
    if (buf_j["connect"] == "true") // something wrong here when implementing the switch
    {
        std::cout << "connection established" << std::endl;
    }
}

void P2pNetwork::update_your_blocks_client(nlohmann::json buf_j)
{
    std::cout << "update_your_blocks client" << std::endl;
    // save blocks to blockchain folder

    nlohmann::json block_j = buf_j["block"];
    std::string block_nr = buf_j["block_nr"];
    // std::cout << "block_s: " << block_j.dump() << std::endl;
    // std::cout << "block_nr: " << block_nr << std::endl;

    merkle_tree mt;
    mt.save_block_to_file(block_j, block_nr);
}

void P2pNetwork::update_your_rocksdb_client(nlohmann::json buf_j)
{
    std::cout << "update_your_rocksdb client" << std::endl;

    std::string key_s = buf_j["key"];
    std::string value_s = buf_j["value"];

    Rocksy* rocksy = new Rocksy("usersdb");
    rocksy->Put(key_s, value_s);
    delete rocksy;
}

void P2pNetwork::update_your_matrices_client(nlohmann::json buf_j)
{
    std::cout << "update_your_matrices client " << std::endl;

    nlohmann::json block_matrix_j = buf_j["bm"];
    
    Poco::BlockMatrix bm;
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

    // TODO do intro_msg_vec and ip_hemail_vec need to be communicated too
    //      or maybe start participating in the network in the next iteration?
}

void P2pNetwork::update_my_blocks_and_rocksdb_client(nlohmann::json buf_j)
{
    std::cout << "update_your_blocks_and_rocksdb client" << std::endl;
    // send blocks to peer

    Protocol proto;
    std::string req_latest_block = buf_j["block_nr"];

    nlohmann::json list_of_blocks_j = proto.get_blocks_from(req_latest_block);
//std::cout << "list_of_blocks_s: " << list_of_blocks_j.dump() << std::endl;

    for (auto& i: list_of_blocks_j.items())
    {
        nlohmann::json msg;
        msg["req"] = "update_your_blocks";
        msg["block_nr"] = i.value()["block_nr"];
        msg["block"] = i.value()["block"];
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

        set_resp_msg_client(msg.dump());
    }
    delete rocksy;
}

void P2pNetwork::update_my_matrices_client(nlohmann::json buf_j)
{
    std::cout << "update_my_matrices client" << std::endl;

    // Update your matrices:
    nlohmann::json msg;
    Poco::BlockMatrix bm;
    nlohmann::json contents_j;
    msg["req"] = "update_my_matrices";
    for (int i = 0; i < bm.get_block_matrix().size(); i++)
    {
        for (int j = 0; j < bm.get_block_matrix().at(i).size(); j++)
        {
            contents_j[std::to_string(i)][std::to_string(j)] = *bm.get_block_matrix().at(i).at(j);
        }
    }
    msg["bm"] = contents_j;
    contents_j.clear();

    set_resp_msg_client(msg.dump());
}

void P2pNetwork::new_peer_client(nlohmann::json buf_j)
{
    // TODO there are 2 new_peer functions that need to be the same, so put them in one function somewhere

    // new_peer
    std::cout << "new_peer: " << std::endl;
    // should read the timestamp of the first new_peer request received
    
    // wait 20 seconds of > 1 MB to create block, to process the timestamp if you are the first new_peer request
    intro_msg_vec_.add_to_intro_msg_vec(buf_j);
    
    if (intro_msg_vec_.get_intro_msg_vec().size() > 2048) // 2048x 512 bit hashes
    {
        // Create block
        Poco::PocoCrowd poco;
        poco.create_and_send_block();

        intro_msg_vec_.reset_intro_msg_vec();
    }
    else if (intro_msg_vec_.get_intro_msg_vec().size() == 1)
    {
        // wait x secs
        // then create block --> don't forget the counter in the search for a coordinator
        // if root_hash == me as coordinator ... connect to all co's
        Poco::Synchronisation* sync = new Poco::Synchronisation();
        std::thread t(&Poco::Synchronisation::get_sleep_and_create_block, sync);
        t.detach();
    }
}

void P2pNetwork::new_co_client(nlohmann::json buf_j)
{
    // send flag to start_crowd function
    std::cout << "new_co: " << buf_j["ip_co"] << std::endl;
    uint32_t peer_ip = buf_j["ip_co"];
    set_ip_new_co(peer_ip); // TODO dunno yet, should be in P2pNetwork
    set_closed_client("new_co");
}

void P2pNetwork::your_full_hash_client(nlohmann::json buf_j)
{
    // my full hash
    std::string full_hash = buf_j["full_hash"];
    std::string prev_hash = buf_j["prev_hash"];
    std::cout << "New peer's full_hash (client): " << full_hash << std::endl;
    std::cout << "New peer's prev_hash (client): " << prev_hash << std::endl;

    // save full_hash
    FullHash fh;
    fh.save_full_hash_to_file(full_hash);

    // save prev_hash
    PrevHash ph;
    ph.save_my_prev_hash_to_file(prev_hash);
    
    nlohmann::json block_j = buf_j["block"];
    std::string req_latest_block_nr = buf_j["block_nr"];

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

    std::cout << "Connection closed by other server, start this server" << std::endl; // TODO here starts duplicate code

    // Disconect from server
    set_closed_client("close_this_conn");
}

void P2pNetwork::hash_comparison_client(nlohmann::json buf_j)
{
    // compare the received hash
    std::cout << "The hash comparison is (client): " << buf_j["hash_comp"] << std::endl;

    set_closed_client("close_this_conn");
}

void P2pNetwork::close_this_conn_client(nlohmann::json buf_j)
{
    // you may close this connection
    std::cout << "Connection closed by other server, start this server (client)" << std::endl;

    set_closed_client("close_this_conn");
}

void P2pNetwork::close_this_conn_and_create_client(nlohmann::json buf_j)
{
    // you may close this connection
    std::cout << "Connection closed by other server, start this server (client) and create" << std::endl;

    set_closed_client("close_this_conn_and_create");
}

void P2pNetwork::set_resp_msg_client(std::string msg)
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
        packet_ = enet_packet_create(resp_msg_.data(), strlen(resp_msg_.data())+1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer_, 0, packet_);
    }
}

std::vector<std::string> P2pNetwork::split(const std::string& str, int splitLength)
{
   int NumSubstrings = str.length() / splitLength;
   std::vector<std::string> ret;

   for (auto i = 0; i < NumSubstrings; i++)
   {
        ret.push_back(str.substr(i * splitLength, splitLength));
   }

   // If there are leftover characters, create a shorter item at the end.
   if (str.length() % splitLength != 0)
   {
        ret.push_back(str.substr(splitLength * NumSubstrings));
   }

   return ret;
}

int P2pNetwork::p2p_client(std::string ip_s, std::string message)
{
    const char *ip = ip_s.c_str();

    int connected=0;

    if (enet_initialize() != 0)
    {
        printf("Could not initialize enet.\n");
        return 0;
    }

    client_ = enet_host_create(NULL, 1, 2, 0, 0);

    if (client_ == NULL)
    {
        printf("Could not create client.\n");
        return 0;
    }

    enet_address_set_host(&address_, ip);
    address_.port = PORT;

    peer_ = enet_host_connect(client_, &address_, 2, 0);

    if (peer_ == NULL)
    {
        printf("Could not connect to server\n");
        return 0;
    }

    if (enet_host_service(client_, &event_, 1000) > 0 && event_.type == ENET_EVENT_TYPE_CONNECT)
    {

        printf("Connection to %s succeeded.\n", ip);
        connected++;

        std::vector<std::string> splitted = split(message, p2p_message::max_body_length);
        for (int i = 0; i < splitted.size(); i++)
        {
            char s[p2p_message::max_body_length];
            strncpy(s, splitted[i].c_str(), sizeof(s));

            p2p_message msg;
            msg.body_length(std::strlen(s));
            std::memcpy(msg.body(), s, msg.body_length());
            i == splitted.size() - 1 ? msg.encode_header(1) : msg.encode_header(0); // 1 indicates end of message eom, TODO perhaps a set_eom_flag(true) instead of an int

            packet_ = enet_packet_create(msg.data(), strlen(msg.data())+1, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(peer_, 0, packet_);
        }
    }
    else
    {
        set_closed_client("closed_conn");

        enet_peer_reset(peer_);
        printf("Could not connect to %s.\n", ip);
        return 0;
    }

    while (1)
    {
        while (enet_host_service(client_, &event_, 1000) > 0)
        {
            switch (event_.type)
            {
                case ENET_EVENT_TYPE_RECEIVE:
                    //puts( (char*) event_.packet->data);
                    sprintf(read_msg_.data(), "%s", (char*) event_.packet->data);
                    do_read_header_client();
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    connected=0;
                    printf("You have been disconnected.\n");
                    return 2;
            }
        }

        if (get_closed_client() == "close_this_conn"
            || get_closed_client() == "close_this_conn_and_create"
            || get_closed_client() == "new_co")
        {
            connected=0;
            enet_peer_disconnect(peer_, 0);
        }
    }

    enet_deinitialize();
}
