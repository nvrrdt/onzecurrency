#include <future>       // std::async, std::future, std::launch
#include <chrono>       // std::chrono::milliseconds
#include <thread>

#include "p2p_network.hpp"
#include "p2p_network_c.hpp"

#include "print_or_log.hpp"

using namespace Crowd;

std::string P2pNetwork::closed_client_ = "";
uint32_t P2pNetwork::ip_new_co_ = 0;
std::vector<std::string> P2pNetwork::client_calls_;

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
        buf_client_ += str_read_msg.substr(0, read_msg_.get_body_length());
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
        req_conversion["new_peer"] =            9;
        req_conversion["new_co"] =              10;
        req_conversion["your_full_hash"] =      11;
        req_conversion["hash_comparison"] =     12;
        req_conversion["close_same_conn"] =     13;
        req_conversion["close_this_conn"] =     14;
        req_conversion["close_this_conn_and_create"] =      15;
        req_conversion["send_first_block"] =    16;
        req_conversion["update_me"] =           17;

        switch (req_conversion[req])
        {
            case 1:     register_for_nat_traversal_client(buf_j);
                        break;
            case 2:     connect_to_nat_client(buf_j);
                        break;
            case 3:     connect_true_client(buf_j);
                        break;
            case 9:     new_peer_client(buf_j);
                        break;
            case 10:    new_co_client(buf_j);
                        break;
            case 11:    your_full_hash_client(buf_j);
                        break;
            case 12:    hash_comparison_client(buf_j);
                        break;
            case 13:    close_same_conn_client(buf_j);
                        break;
            case 14:    close_this_conn_client(buf_j);
                        break;
            case 15:    close_this_conn_and_create_client(buf_j);
                        break;
            case 16:    send_first_block_received_client(buf_j);
                        break;
            case 17:    update_me_client(buf_j);
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

        Common::Print_or_log pl;
        pl.handle_print_or_log({"Ack for registering this client to a server"});
    }
}

void P2pNetwork::connect_to_nat_client(nlohmann::json buf_j)
{
    if (buf_j["connect"] == "ok")
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"connect = ok"});
        
        nlohmann::json message_j;
        message_j["connect"] = "true";

        // Establishing NAT Traversal
        // TODO: needs to be tested that there is really a connection between the two peers
        if (buf_j["id_from"] == "nvrrdt_from") // TODO: change nvrrdt to my_id/my_hash/my_ip
        {

            pl.handle_print_or_log({"message send to id_to from id_from"});

            std::string peer_ip = buf_j["ip_to"];
            std::string message = message_j.dump();
            std::string pub_key = "pub_key";
            p2p_client(peer_ip, message);
            p2p_server();
        }
        else
        {
            pl.handle_print_or_log({"message send to id_from from id_to"});
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
        Common::Print_or_log pl;
        pl.handle_print_or_log({"connection established"});
    }
}

void P2pNetwork::new_peer_client(nlohmann::json buf_j)
{
    // TODO there are 2 new_peer functions that need to be the same, so put them in one function somewhere

    // new_peer
    Common::Print_or_log pl;
    pl.handle_print_or_log({"new_peer client:"});
    // should read the timestamp of the first new_peer request received
    
    // wait 20 seconds or > 1 MB to create block, to process the timestamp if you are the first new_peer request
    intro_msg_vec_.add_to_intro_msg_vec(buf_j);
    
    if (intro_msg_vec_.get_intro_msg_vec().size() > 2048) // 2048x 512 bit hashes
    {
        // Create block
        Poco::PocoCrowd poco;
        poco.create_prel_blocks();

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

    P2p p2p;
    enet_uint32 peer_ip = buf_j["ip_co"];
    std::string peer_ip_quad;
    p2p.number_to_ip_string(peer_ip, peer_ip_quad);
    Common::Print_or_log pl;
    pl.handle_print_or_log({"new_co:", peer_ip_quad});

    set_ip_new_co(peer_ip); // TODO dunno yet, should be in P2pNetwork
    set_closed_client("new_co");
}

void P2pNetwork::your_full_hash_client(nlohmann::json buf_j)
{
    // my full hash
    std::string full_hash = buf_j["full_hash"];
    std::string prev_hash = buf_j["prev_hash"];

    Common::Print_or_log pl;
    pl.handle_print_or_log({"New peer's full_hash (client): ", full_hash});
    pl.handle_print_or_log({"New peer's prev_hash (client): ", prev_hash});

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

Crowd::Protocol proto;
std::map<int, std::string> partsx = proto.partition_in_buckets(full_hash, full_hash);
int k;
std::string v;
for (auto &[k, v] : partsx)
{
    pl.handle_print_or_log({"___000112 your_full_hash client", std::to_string(k), v});
}

    pl.handle_print_or_log({"Connection closed by other server, start this server"}); // TODO here starts duplicate code

    // Disconect from server
    set_closed_client("close_this_conn");
}

void P2pNetwork::hash_comparison_client(nlohmann::json buf_j)
{
    // compare the received hash
    Common::Print_or_log pl;
    pl.handle_print_or_log({"The hash comparison is (client):", (buf_j["hash_comp"]).dump()});

    set_closed_client("close_this_conn_and_create");
}

void P2pNetwork::close_same_conn_client(nlohmann::json buf_j)
{
    // you may close this connection
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Connection closed by other server, start this server (same client)"});

    set_closed_client("close_same_conn");
}

void P2pNetwork::close_this_conn_client(nlohmann::json buf_j)
{
    // you may close this connection
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Connection closed by other server, start this server (client)"});

    set_closed_client("close_this_conn");
}

void P2pNetwork::close_this_conn_and_create_client(nlohmann::json buf_j)
{
    // you may close this connection
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Connection closed by other server, start this server (client) and create"});

    set_closed_client("close_this_conn_and_create");
}

void P2pNetwork::send_first_block_received_client(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Received first block (client)"});

    nlohmann::json first_block_j = buf_j["block"];
    std::string block_nr = "0";

    merkle_tree mt;
    mt.save_block_to_file(first_block_j, block_nr);
}

void P2pNetwork::update_me_client(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Update_you: send all blocks, rocksdb and matrices to server (client)"});

    std::string req_latest_block = buf_j["block_nr"];

    nlohmann::json msg;
    msg["req"] = "update_you";
    Protocol proto;

    // Update blockchain
    msg["blocks"] = proto.get_blocks_from(req_latest_block);

    nlohmann::json list_of_users_j = nlohmann::json::parse(proto.get_all_users_from(req_latest_block)); // TODO: there are double parse/dumps everywhere
                                                                                                        // maybe even a stack is better ...
    // Update rocksdb
    nlohmann::json rdb;
    Rocksy* rocksy = new Rocksy("usersdbreadonly");
    for (auto& user : list_of_users_j)
    {
        nlohmann::json usr;
        std::string u = user;
        nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(u));
        usr = {u: value_j};
        rdb.push_back(usr);
    }
    delete rocksy;

    msg["rocksdb"] = rdb;

    // Update matrices
    Poco::BlockMatrix bm;
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
    msg["bm"] = contents_j;
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
    msg["imm"] = contents_j;
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
    msg["iah"] = contents_j;
    contents_j.clear();

    // Update intro_msg_vec and ip_hemail_vec
    msg["imv"];
    for (auto& el: intro_msg_vec_.get_intro_msg_vec())
    {
        msg["imv"].push_back(*el);
    }

    msg["ihv"];
    for (auto& el: ip_hemail_vec_.get_all_ip_hemail_vec())
    {
        msg["ihv"][std::to_string((*el).first)] = (*el).second;
    }

    set_resp_msg_client(msg.dump());
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

    Common::Print_or_log pl;

    for (;;)
    {
        std::future<bool> cts = std::async(std::launch::async, &P2pNetwork::is_connected_to_server, this, ip_s);
        std::future<bool> cc = std::async(std::launch::async, &P2pNetwork::has_connected_client, this, ip_s);
        bool b_cts = cts.get(), b_cc = cc.get();
pl.handle_print_or_log({"__00654", "p2p_client pre", std::to_string(b_cts), std::to_string(b_cc)});
        if (!b_cts && !b_cc)
        {
pl.handle_print_or_log({"__00654", "p2p_client go_for_it"});
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    add_to_client_calls(ip_s);

    if (enet_initialize() != 0)
    {
        pl.handle_print_or_log({"Could not initialize enet."});
        return 0;
    }

    client_ = enet_host_create(NULL, 105, 2, 0, 0);

    if (client_ == NULL)
    {
        pl.handle_print_or_log({"Could not create client."});
        return 0;
    }

    enet_address_set_host(&address_, ip);
    address_.port = PORT;

    peer_ = enet_host_connect(client_, &address_, 2, 0);

    if (peer_ == NULL)
    {
        pl.handle_print_or_log({"Could not connect to server"});        
        return 0;
    }

    if (enet_host_service(client_, &event_, 10000) > 0 && event_.type == ENET_EVENT_TYPE_CONNECT)
    {
        pl.handle_print_or_log({"Connection to", ip, "succeeded."});
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
        pl.handle_print_or_log({"Could not connect to", ip});

        remove_from_client_calls(ip_s);

        return 0;
    }

    Crowd::P2p p2p;
    std::string ip_client;
    while (1)
    {
        while (enet_host_service(client_, &event_, 50) > 0)
        {
            switch (event_.type)
            {
                case ENET_EVENT_TYPE_RECEIVE:
                    //puts( (char*) event_.packet->data);
                    sprintf(read_msg_.data(), "%s", (char*) event_.packet->data);
                    do_read_header_client();
                    enet_packet_destroy(event_.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    connected=0;
                    pl.handle_print_or_log({"You have been disconnected.", ip_client});

                    return 2;
            }
        }

        if (get_closed_client() == "close_same_conn"
            || get_closed_client() == "close_this_conn"
            || get_closed_client() == "close_this_conn_and_create"
            || get_closed_client() == "new_co")
        {
            connected=0;
            enet_peer_disconnect(peer_, 0);

            remove_from_client_calls(ip_s);
        }
    }

    enet_host_destroy(client_);
    enet_deinitialize();

    return 0;
}

// Sometimes the server stops when 2 peers are simultaneously trying to conenect to each other
// Solution is to halt the slowest p2p_client, there's a reaction at connection at server side
bool P2pNetwork::is_connected_to_server(std::string ip_s)
{
    for (int i = 0; i < get_connected_to_server().size(); i++)
    {
        if (ip_s == get_connected_to_server().at(i))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));

            return true;
        }
    }

    return false;
}

// sometimes there are two p2p_client() calls at the same time by two threads
// ip_s in vector and at disconnect out of vector
bool P2pNetwork::has_connected_client(std::string ip_s)
{
    for (int j = 0; j < get_client_calls().size(); j++)
    {
        if (ip_s == get_client_calls().at(j))
        {
            return true;
        }
    }

    return false;
}