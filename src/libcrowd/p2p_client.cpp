#include "p2p_network.hpp"
#include "p2p_network_c.hpp"

#include "print_or_log.hpp"

#include <cstdlib>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "p2p_message.hpp"

using boost::asio::ip::tcp;
using namespace Crowd;

std::string P2pNetwork::closed_client_ = "";
std::string P2pNetwork::ip_new_co_ = 0;

void P2pClient::handle_read_client(p2p_message read_msg_client)
{
    if ( !read_msg_client.get_eom_flag())
    {
        std::string str_read_msg(read_msg_client.body());
        buf_client_ += str_read_msg.substr(0, read_msg_client.get_body_length());
    }
    else
    {
        // process json message
        std::string str_read_msg(read_msg_client.body());
        buf_client_ += str_read_msg.substr(0, read_msg_client.get_body_length());
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

void P2pClient::register_for_nat_traversal_client(nlohmann::json buf_j)
{
    if (buf_j["register"] == "ack") // something wrong here when implementing the switch
    {
        // TODO: what if there was no response from the server?

        Common::Print_or_log pl;
        pl.handle_print_or_log({"Ack for registering this client to a server"});
    }
}

void P2pClient::connect_to_nat_client(nlohmann::json buf_j)
{
    if (buf_j["connect"] == "ok")
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"connect = ok"});
        
        nlohmann::json message_j;
        message_j["connect"] = "true";

        // Establishing NAT Traversal
        // TODO: needs to be tested that there is really a connection between the two peers
        P2pNetwork pn;
        if (buf_j["id_from"] == "nvrrdt_from") // TODO: change nvrrdt to my_id/my_hash/my_ip
        {

            pl.handle_print_or_log({"message send to id_to from id_from"});

            std::string peer_ip = buf_j["ip_to"];
            std::string message = message_j.dump();
            std::string pub_key = "pub_key";
            pn.p2p_client(peer_ip, message);
            pn.p2p_server();
        }
        else
        {
            pl.handle_print_or_log({"message send to id_from from id_to"});
            std::string peer_ip = buf_j["ip_from"];
            std::string message = message_j.dump();
            std::string pub_key = "pub_key";
            pn.p2p_client(peer_ip, message);
            pn.p2p_server();
        }
    }
}

void P2pClient::connect_true_client(nlohmann::json buf_j)
{
    if (buf_j["connect"] == "true") // something wrong here when implementing the switch
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"connection established"});
    }
}

void P2pClient::new_peer_client(nlohmann::json buf_j)
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

void P2pClient::new_co_client(nlohmann::json buf_j)
{
    // send flag to start_crowd function

    P2p p2p;
    std::string peer_ip = buf_j["ip_co"];
    Common::Print_or_log pl;
    pl.handle_print_or_log({"new_co:", peer_ip});

    close();

    P2pNetwork pn;
    pn.set_ip_new_co(peer_ip); // TODO dunno yet, should be in P2pNetwork
    pn.set_closed_client("new_co");
}

void P2pClient::your_full_hash_client(nlohmann::json buf_j)
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
    close();
    P2pNetwork pn;
    pn.set_closed_client("close_this_conn");
}

void P2pClient::hash_comparison_client(nlohmann::json buf_j)
{
    // compare the received hash
    Common::Print_or_log pl;
    pl.handle_print_or_log({"The hash comparison is (client):", (buf_j["hash_comp"]).dump()});

    close();
    P2pNetwork pn;
    pn.set_closed_client("close_this_conn_and_create");
}

void P2pClient::close_same_conn_client(nlohmann::json buf_j)
{
    // you may close this connection
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Connection closed by other server, start this server (same client)"});

    close();
    P2pNetwork pn;
    pn.set_closed_client("close_same_conn");
}

void P2pClient::close_this_conn_client(nlohmann::json buf_j)
{
    // you may close this connection
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Connection closed by other server, start this server (client)"});

    close();
    P2pNetwork pn;
    pn.set_closed_client("close_this_conn");
}

void P2pClient::close_this_conn_and_create_client(nlohmann::json buf_j)
{
    // you may close this connection
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Connection closed by other server, start this server (client) and create"});

    close();
    P2pNetwork pn;
    pn.set_closed_client("close_this_conn_and_create");
}

void P2pClient::send_first_block_received_client(nlohmann::json buf_j)
{
    Common::Print_or_log pl;
    pl.handle_print_or_log({"Received first block (client)"});

    nlohmann::json first_block_j = buf_j["block"];
    std::string block_nr = "0";

    merkle_tree mt;
    mt.save_block_to_file(first_block_j, block_nr);
}

void P2pClient::update_me_client(nlohmann::json buf_j)
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
        msg["ihv"][(*el).first] = (*el).second;
    }

    set_resp_msg_client(msg.dump());
}

void P2pClient::set_resp_msg_client(std::string msg)
{
    P2pNetwork pn;
    std::vector<std::string> splitted = pn.split(msg, p2p_message::max_body_length);
    p2p_message resp_msg_client;
    for (int i = 0; i < splitted.size(); i++)
    {
        char s[p2p_message::max_body_length];
        strncpy(s, splitted[i].c_str(), sizeof(s));

        resp_msg_client.body_length(std::strlen(s));
        std::memcpy(resp_msg_client.body(), s, resp_msg_client.body_length());
        i == splitted.size() - 1 ? resp_msg_client.encode_header(1) : resp_msg_client.encode_header(0); // 1 indicates end of message eom, TODO perhaps a set_eom_flag(true) instead of an int

        write(resp_msg_client);
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

P2pClient::P2pClient(boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints)
    : io_context_(io_context), socket_(io_context)
{
    do_connect(endpoints);
}

void P2pClient::write(const p2p_message &msg)
{
    boost::asio::post(io_context_,
                        [this, msg]()
                        {
                            bool write_in_progress = !write_msgs_.empty();
                            write_msgs_.push_back(msg);
                            if (!write_in_progress)
                            {
                                do_write();
                            }
                        });
}

void P2pClient::close()
{
    boost::asio::post(io_context_, [this]()
                        { socket_.close(); });
}

void P2pClient::do_connect(const tcp::resolver::results_type &endpoints)
{
    boost::asio::async_connect(socket_, endpoints,
                                [this](boost::system::error_code ec, tcp::endpoint)
                                {
                                    if (!ec)
                                    {
                                        do_read_header();
                                    }
                                });
}

void P2pClient::do_read_header()
{
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_msg_.data(), p2p_message::header_length),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec && read_msg_.decode_header())
                                {
                                    do_read_body();
                                }
                                else
                                {
                                    socket_.close();
                                }
                            });
}

void P2pClient::do_read_body()
{
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec)
                                {
                                    std::cout.write(read_msg_.body(), read_msg_.body_length());
                                    std::cout << "\n";
                                    handle_read_client(read_msg_);
                                    do_read_header();
                                }
                                else
                                {
                                    socket_.close();
                                }
                            });
}

void P2pClient::do_write()
{
    boost::asio::async_write(socket_,
                                boost::asio::buffer(write_msgs_.front().data(),
                                                    write_msgs_.front().length()),
                                [this](boost::system::error_code ec, std::size_t /*length*/)
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
                                        socket_.close();
                                    }
                                });
}



int P2pNetwork::p2p_client(std::string ip_s, std::string message)
{
    try
    {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(ip_s, PORT);
        P2pClient c(io_context, endpoints);

        std::thread t([&io_context]()
                      { io_context.run(); });

        P2pNetwork pn;
        std::vector<std::string> splitted = pn.split(message, p2p_message::max_body_length);
        p2p_message resp_msg;
        for (int i = 0; i < splitted.size(); i++)
        {
            char s[p2p_message::max_body_length];
            strncpy(s, splitted[i].c_str(), sizeof(s));

            resp_msg.body_length(std::strlen(s));
            std::memcpy(resp_msg.body(), s, resp_msg.body_length());
            i == splitted.size() - 1 ? resp_msg.encode_header(1) : resp_msg.encode_header(0); // 1 indicates end of message eom, TODO perhaps a set_eom_flag(true) instead of an int

            c.write(resp_msg);
        }

        c.close();
        t.join();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception client: " << e.what() << "\n";
    }

    return 0;
}