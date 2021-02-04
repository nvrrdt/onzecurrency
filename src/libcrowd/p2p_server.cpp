#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "p2p_message.hpp"
#include "p2p.hpp"
#include "json.hpp"
#include "ip_peers.hpp"
#include "auth.hpp"
#include "crypto_ecdsa.hpp"
#include "crypto_shab58.hpp"

using namespace Crowd;
using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<p2p_message> p2p_message_queue;

//----------------------------------------------------------------------

class p2p_participant
{
public:
    virtual ~p2p_participant() {}
    virtual void deliver(const p2p_message &msg) = 0;
    void set_id(std::string id) {
        id_ = id;
    }
    std::string get_id() {
        return id_;
    }
    void set_find_id(std::string id) {
        find_id_ = id;
    }
    std::string get_find_id() {
        return find_id_;
    }
private:
    std::string id_;
    std::string find_id_;
};

typedef std::shared_ptr<p2p_participant> p2p_participant_ptr;

//----------------------------------------------------------------------

class p2p_room
{
public:
    void join(p2p_participant_ptr participant)
    {
        std::cout << "participant: " << participant << std::endl;
        participants_.insert(participant);
        for (auto msg : recent_msgs_)
            participant->deliver(msg);
    }

    void leave(p2p_participant_ptr participant)
    {
        participants_.erase(participant);
    }

    void deliver(const p2p_message &msg)
    {
        recent_msgs_.push_back(msg);
        while (recent_msgs_.size() > max_recent_msgs)
            recent_msgs_.pop_front();

        for (auto participant : participants_)
            if (participant->get_id() == participant->get_find_id())
            {
                participant->deliver(msg);
            }
    }

private:
    std::set<p2p_participant_ptr> participants_;
    enum
    {
        max_recent_msgs = 100
    };
    p2p_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

class p2p_session
    : public p2p_participant,
      public std::enable_shared_from_this<p2p_session>
{
public:
    p2p_session(tcp::socket socket, p2p_room &room)
        : socket_(std::move(socket)),
          room_(room)
    {
    }

    void start()
    {
        do_read_header();
    }

    void deliver(const p2p_message &msg)
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            do_write();
        }
    }

private:
    void do_read_header()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), p2p_message::header_length),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                    if (!ec && read_msg_.decode_header())
                                    {
                                        do_read_body();
                                    }
                                    else
                                    {
                                        room_.leave(shared_from_this());
                                    }
                                });
    }

    void do_read_body()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                    if (!ec)
                                    {
                                        handle_read(ec);
                                        do_read_header();
                                    }
                                    else
                                    {
                                        room_.leave(shared_from_this());
                                    }
                                });
    }

    void handle_read(boost::system::error_code ec)
    {
        if ( !read_msg_.get_eom_flag()) {
            std::string str_read_msg(read_msg_.body());
            buf_ += str_read_msg;
            
        } else {
            // process json message
            std::string str_read_msg(read_msg_.body());
            buf_ += str_read_msg.substr(0, read_msg_.get_body_length());
            
            nlohmann::json buf_j = nlohmann::json::parse(buf_);
            if (ec)
                throw boost::system::system_error(ec); // Some other error.
            else if (buf_j["req"] == "register")
            {
                shared_from_this()->set_id(buf_j["id"]);        // needs to be worked out, it's possible that all peers get all messages
                shared_from_this()->set_find_id(buf_j["id"]);   // while the two connecting peers should only get their messages
                room_.join(shared_from_this());

                nlohmann::json resp_j;
                resp_j["register"] = "ack";

                set_resp_msg(resp_j.dump());
                room_.deliver(resp_msg_);

                std::cout << "Ack for registering client is confirmed" << std::endl;
            }
            else if (buf_j["req"] == "connect")
            {
                std::cout << "connection request for a peer behind a nat" << std::endl;
                
                //send back to peer who wants to connect
                room_.join(shared_from_this());

                nlohmann::json resp_j;
                resp_j["req"] = "connect";
                resp_j["connect"] = "ok";
                resp_j["id_to"] = buf_j["id_to"];
                resp_j["ip_to"] = buf_j["ip_to"];
                resp_j["id_from"] = buf_j["id_from"];
                resp_j["ip_from"] = buf_j["ip_from"];

                set_resp_msg(resp_j.dump()); 
                room_.deliver(resp_msg_);
 
                room_.leave(shared_from_this());
            } 
            else if (buf_j["req"] == "intro_peer")
            {
                // process buf_j["hash_of_req"] to find ip of the peer who should update you
                std::string co_from_req = buf_j["full_hash_co"];
                std::string email_of_req = buf_j["email_of_req"];
                std::string prev_hash_req = buf_j["prev_hash_of_req"];
                std::string pub_key = buf_j["pub_key"];
                std::string signature = buf_j["signature"];
                std::string req_latest_block = buf_j["latest_block"];

                Shab58 s;
                std::string full_hash_req =  s.create_base58_hash(email_of_req + prev_hash_req);

                std::cout << "email: " << email_of_req << std::endl;

                nlohmann::json to_verify_j;
                to_verify_j["pub_key"] = pub_key;
                to_verify_j["email"] = email_of_req;

                Ecdsa e;
                std::string to_verify_s = to_verify_j.dump();
                if (e.verify(to_verify_s, signature, pub_key))
                {
                    std::cout << "verified" << std::endl;
                    Poco p;
                    std::string to_find_co = s.create_base58_hash(full_hash_req);
                    std::string co_from_this_db = p.FindChosenOne(to_find_co);
                    std::cout << "co_from_this_db: " << co_from_this_db << std::endl;
                    std::cout << "co_from_req: " << co_from_req << std::endl;
                    // Do the chosen_one communicated correspond to the chosen_one lookup in db?
                    if (co_from_this_db == co_from_req || co_from_req == "0")
                    {
                        Auth a;
                        std::string my_full_hash = a.get_my_full_hash();
                        std::cout << "my_full_hash: " << my_full_hash << std::endl;
                        std::cout << "co_from_req: " << co_from_req << std::endl;
                        std::cout << "co_from_this_db: " << co_from_this_db << std::endl;
                        if (co_from_this_db == my_full_hash) // TODO: an eta should be introduced for when someone enters/leaves the network
                        {
                            // I'm the chosen one
                            std::cout << "I'm the chosen one!" << std::endl;

                            Protocol proto;
                            std::string my_latest_block = proto.latest_block();
                            std::cout << "My latest block: " << my_latest_block << std::endl;

                            if (req_latest_block < my_latest_block)
                            {
                                // TODO: upload blockchain to the requester starting from latest block
                            }
                            else if (req_latest_block > my_latest_block)
                            {
                                // TODO: update your own blockchain
                            }

                            // for the server: layer_management needed: assemble all the chosen ones in rocksdb,
                            // then create clients to them all with new_peer message
                        }
                        else
                        {
                            // I'm not the chosen one, reply with get_new_co
                            std::cout << "I'm NOT the chosen one!" << std::endl;
                        }
                    }
                    else
                    {
                        // Req does't yet exist in rocksdb or there's another chosen one, reply with error
                        std::cerr << "ERROR: no full_hash_req was sent or chosen_one is someone else!" << std::endl;
                    }
                }
                else
                {
                    std::cout << "failed verification" << std::endl;
                    room_.leave(shared_from_this());
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

                //     CreateBlock cb(email_of_peer, hash_of_peer); // moet bij new_peer
                //     std::cout << "Is this reached? " << hash_of_peer << std::endl;
                //     // if cb ok: update blockchain and update rocksdb will be received through the chosen one's
                // }
                // else
                // {
                //     // If there are more peers in the ip_list ...
                // }
            }
        }
    }

    void set_resp_msg(std::string msg)
    {
        char c[p2p_message::max_body_length];
        strncpy(c, msg.c_str(), sizeof(c));
        resp_msg_.body_length(std::strlen(c));
        std::memcpy(resp_msg_.body(), c, resp_msg_.body_length());
        resp_msg_.encode_header(1); // TODO a 0 arg = not eom, should also be implemented for when > max_body_length, see ?: in p2p_client
    }

    void do_write()
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                     if (!ec)
                                     {
                                         write_msgs_.pop_front();
                                         if (!write_msgs_.empty())
                                         {
                                             std::cout << "sending response" << std::endl;
                                             do_write();
                                         }
                                     }
                                     else
                                     {
                                         room_.leave(shared_from_this());
                                     }
                                 });
    }

    tcp::socket socket_;
    p2p_room &room_;
    p2p_message read_msg_;
    p2p_message_queue write_msgs_;
    std::string buf_;
    p2p_message resp_msg_;
};

//----------------------------------------------------------------------

class p2p_server
{
public:
    p2p_server(boost::asio::io_context &io_context,
                const tcp::endpoint &endpoint)
        : acceptor_(io_context, endpoint)
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec)
                {
                    std::make_shared<p2p_session>(std::move(socket), room_)->start();
                }

                do_accept();
            });
    }

    tcp::acceptor acceptor_;
    p2p_room room_;
};

//----------------------------------------------------------------------

int Tcp::server()
{
    try
    {
        boost::asio::io_context io_context;

        std::list<p2p_server> servers;
        tcp::endpoint endpoint(tcp::v4(), 1975);
        servers.emplace_back(io_context, endpoint);

        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}