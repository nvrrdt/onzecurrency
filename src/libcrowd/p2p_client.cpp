#include "p2p_network.hpp"

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
    if ( !read_msg_.get_eom_flag()) {
        std::string str_read_msg(read_msg_.body());
        buf_client_ += str_read_msg;
    } else {
        // process json message
        std::string str_read_msg(read_msg_.body());
        buf_client_ += str_read_msg.substr(0, read_msg_.get_body_length());

        nlohmann::json buf_j = nlohmann::json::parse(buf_client_);
        if (buf_j["register"] == "ack")
        {
            // TODO: what if there was no response from the server?

            std::cout << "Ack for registering this client to a server" << std::endl;
        }
        else if (buf_j["req"] == "connect")
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
        else if (buf_j["connect"] == "true")
        {
            std::cout << "connection established" << std::endl;
        }
        else if (buf_j["req"] == "update_your_blocks")
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
        else if (buf_j["req"] == "update_your_rocksdb")
        {
            std::cout << "update_your_rocksdb client" << std::endl;

            std::string key_s = buf_j["key"];
            std::string value_s = buf_j["value"];

            Rocksy* rocksy = new Rocksy();
            rocksy->Put(key_s, value_s);
            delete rocksy;
        }
        else if (buf_j["req"] == "update_my_blocks_and_rocksdb")
        {
            std::cout << "update_your_blocks_and_rocksdb client" << std::endl;
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
            Rocksy* rocksy = new Rocksy();
            for (auto& user : list_of_users_j)
            {
                nlohmann::json msg;
                msg["req"] = "update_your_rocksdb";
                msg["key"] = user;

                std::string u = user.dump();
                std::string value = rocksy->Get(u);
                msg["value"] = value;

                set_resp_msg_client(msg.dump());
            }
            delete rocksy;
        }
        else if (buf_j["req"] == "new_peer")
        {
            // TODO there are 2 new_peer functions that need to be the same, so put them in one function somewhere

            // new_peer
            std::cout << "new_peer: " << std::endl;
            // should read the timestamp of the first new_peer request received
            
            // wait 20 seconds of > 1 MB to create block, to process the timestamp if you are the first new_peer request
            message_j_vec_.add_to_message_j_vec(buf_j);
            
            if (message_j_vec_.get_message_j_vec().size() > 2048) // 2048x 512 bit hashes
            {
                // Create block
                Poco poco;
                poco.create_and_send_block();

                message_j_vec_.reset_message_j_vec();
            }
            else if (message_j_vec_.get_message_j_vec().size() == 1)
            {
                // wait 20 secs
                // then create block --> don't forget the counter in the search for a coordinator
                // if root_hash == me as coordinator ... connect to all co's
                std::thread t(&P2pNetwork::get_sleep_and_create_block_client, this);
                t.detach();
            }
        }
        else if (buf_j["req"] == "new_co")
        {
            // send flag to start_p2p function
            std::cout << "new_co: " << std::endl;
            uint32_t peer_ip = buf_j["ip_co"];
            set_ip_new_co(peer_ip); // TODO dunno yet, should be in P2pNetwork
            set_closed_client("new_co");
        }
        else if (buf_j["req"] == "your_full_hash")
        {
            // my full hash
            std::string full_hash = buf_j["full_hash"];
            std::cout << "New peer's full_hash (client): " << full_hash << std::endl;

            // save full_hash
            FullHash fh;
            fh.save_full_hash_to_file(full_hash);
            
            nlohmann::json block_j = buf_j["block"];
            std::string req_latest_block_nr = buf_j["block_nr"];

            merkle_tree mt;
            mt.save_block_to_file(block_j,req_latest_block_nr);

            // Put in rocksdb
            for (auto &[key, value] : buf_j["rocksdb"].items())
            {
                std::string key_s = value["full_hash"];
                std::string value_s = value.dump();

                Rocksy* rocksy = new Rocksy();
                rocksy->Put(key_s, value_s);
                delete rocksy;
            }

            std::cout << "Connection closed by other server, start this server" << std::endl; // TODO here starts duplicate code

            // Disconect from server
            set_closed_client("close_this_conn");

            std::packaged_task<void()> task1([] {
                P2pNetwork pn;
                pn.p2p_server();
            });
            // Run task on new thread.
            std::thread t1(std::move(task1));
            t1.join();
        }
        else if (buf_j["req"] == "hash_comparison")
        {
            // compare the received hash
            std::cout << "The hash comparison is (client): " << buf_j["hash_comp"] << std::endl;

            set_closed_client("close_this_conn");
        }
        else if (buf_j["req"] == "close_this_conn")
        {
            // you may close this connection
            std::cout << "Connection closed by other server, start this server (client)" << std::endl;

            set_closed_client("close_this_conn");

            std::packaged_task<void()> task1([] {
                P2pNetwork pn;
                pn.p2p_server();
            });
            // Run task on new thread.
            std::thread t1(std::move(task1));
            t1.join();
        }
        else if (buf_j["req"] == "close_this_conn_without_server")
        {
            // you may close this connection
            std::cout << "Connection closed by other server, no server start" << std::endl;

            set_closed_client("close_this_conn");
        }

        buf_client_ = ""; // reset buffer, otherwise nlohmann receives an incorrect string
    }
}

void P2pNetwork::get_sleep_and_create_block_client() // TODO in p2p_server is also this function, they should be merged as they need to be the same
{
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "message_j_vec.size() in Poco: " << message_j_vec_.get_message_j_vec().size() << std::endl;

    Poco poco;
    poco.create_and_send_block();

    // TODO look into p2p_session for the same function and adapt accordingly
    // this function is not yet used, it starts getting used when nat traversal is introcuded

    std::cout << "Block created client!!" << std::endl;
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

        if (get_closed_client() == "close_this_conn" || get_closed_client() == "new_co")
        {
            connected=0;
            enet_peer_disconnect(peer_, 0);
        }
    }

    enet_deinitialize();
}