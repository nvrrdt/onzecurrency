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
#include "crypto.hpp"
#include "prev_hash.hpp"
#include "merkle_tree.hpp"

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
        {
            if (participant->get_id() == participant->get_find_id())
            {
                participant->deliver(msg);
            }
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
            buf_ += str_read_msg.substr(0, read_msg_.get_body_length());
            
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
                 
                room_.leave(shared_from_this());
            } 
            else if (buf_j["req"] == "intro_peer")
            {
                // process buf_j["hash_of_req"] to find ip of the peer who should update you
                std::string co_from_req = buf_j["full_hash_co"];
                std::string email_of_req = buf_j["email_of_req"];
                std::string prev_hash_req = buf_j["prev_hash_of_req"];
                std::string ecdsa_pub_key_s = buf_j["ecdsa_pub_key"];
                std::string rsa_pub_key = buf_j["rsa_pub_key"];
                std::cout << "1p: " << buf_j.dump() << std::endl;
                std::string signature = buf_j["signature"];
                std::cout << "2p: " << std::endl;
                std::string req_latest_block = buf_j["latest_block"];

                Crypto* crypto = new Crypto();
                std::string email__prev_hash_app = email_of_req + prev_hash_req;
                std::string full_hash_req =  crypto->bech32_encode_sha256(email__prev_hash_app);

                Poco* poco = new Poco();
                if (poco->Get(full_hash_req) == "")
                {
                    delete poco;

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
                        std::cout << "verification1p succeeded: " << std::endl;
                        std::cout << "ecdsa_p_key: " << "X" << ecdsa_pub_key_s << "X" << std::endl;
                        std::cout << "to_sign_s: " << "X" << to_verify_s << "X" << std::endl;
                        std::cout << "base64_signature: " << "X" << signature << "X" << std::endl; 
                        std::cout << "signature_bin: " << "X" << signature_bin << "X" << std::endl; 

                        std::cout << "verified" << std::endl;
                        Poco* poco = new Poco();
                        std::string to_find_co = crypto->bech32_encode_sha256(full_hash_req);
                        std::string co_from_this_db = poco->FindChosenOne(to_find_co);
                        delete poco;
                        std::cout << "co_from_this_db: " << co_from_this_db << std::endl;
                        std::cout << "co_from_req: " << co_from_req << std::endl;
                        // Do the chosen_one communicated correspond to the chosen_one lookup in db?
                        if (co_from_this_db == co_from_req || co_from_req == "0")
                        {
                            Auth a;
                            std::string my_full_hash = a.get_my_full_hash();
                            std::cout << "my_full_hash: " << my_full_hash << std::endl;

                            Protocol proto;
                            std::string my_latest_block = proto.latest_block();
                            std::cout << "My latest block: " << my_latest_block << std::endl;
                            std::cout << "Req latest block: " << req_latest_block << std::endl;

                            room_.join(shared_from_this());
                            
                            if (req_latest_block < my_latest_block || req_latest_block == "no blockchain present in folder")
                            {
                                // TODO: upload blockchain to the requester starting from latest block
                                // send latest block to peer
                                nlohmann::json list_of_blocks_j = nlohmann::json::parse(proto.get_blocks_from(req_latest_block));

                                // the fist block in the blockchain directory contains weird escape characters .......
                                nlohmann::json block_j = list_of_blocks_j[0]["block"];
                                std::cout << "block_j: " << block_j << std::endl;
                                nlohmann::json msg;
                                msg["req"] = "new_block";
                                msg["block_nr"] = "0";
                                msg["block"] = block_j;
                                set_resp_msg(msg.dump());
                            }
                            else if (req_latest_block > my_latest_block)
                            {
                                // TODO: update your own blockchain
                            }

                            // for the server: layer_management needed: assemble all the chosen ones in rocksdb,
                            // then create clients to them all with new_peer message

                            Poco* poco = new Poco();
                            if (poco->TotalAmountOfPeers() == 1)
                            {
                                delete poco;

                                // create_block ...
                                // resp_msg_ = ...
                                // inform room_.deliver(resp_msg_);
                                nlohmann::json to_block_j, entry_tx_j, entry_transactions_j, exit_tx_j, exit_transactions_j, rocksdb_j;
                                
                                std::string hash_email = crypto->bech32_encode_sha256(email_of_req);
                                PrevHash ph;
                                std::string prev_hash = ph.get_last_prev_hash_from_blocks();
                                std::cout << "prev_hash: " << prev_hash << std::endl;
                                std::string hash_email_prev_hash_app = hash_email + prev_hash;
                                std::string full_hash_of_new_peer = crypto->bech32_encode_sha256(hash_email_prev_hash_app);
                                
                                to_block_j["full_hash"] = full_hash_of_new_peer;
                                to_block_j["ecdsa_pub_key"] = ecdsa_pub_key_s;
                                to_block_j["rsa_pub_key"] = rsa_pub_key;

                                std::shared_ptr<std::stack<std::string>> s_shptr = make_shared<std::stack<std::string>>();
                                s_shptr->push(to_block_j.dump());
                                merkle_tree mt;
                                s_shptr = mt.calculate_root_hash(s_shptr);
                                entry_tx_j["full_hash"] = to_block_j["full_hash"];
                                entry_tx_j["ecdsa_pub_key"] = to_block_j["ecdsa_pub_key"];
                                entry_tx_j["rsa_pub_key"] = to_block_j["rsa_pub_key"];
                                entry_transactions_j.push_back(entry_tx_j);
                                exit_tx_j["full_hash"] = "";
                                exit_transactions_j.push_back(exit_tx_j);
                                std::string datetime = mt.time_now();
                                std::string root_hash_data = s_shptr->top();
                                nlohmann::json block_j = mt.create_block(datetime, root_hash_data, entry_transactions_j, exit_transactions_j);
                                std::string block_s = mt.save_block_to_file(block_j);

                                // Update rocksdb
                                rocksdb_j["version"] = "O.1";
                                rocksdb_j["ip"] = ip_of_peer_;
                                rocksdb_j["server"] = true;
                                rocksdb_j["fullnode"] = true;
                                rocksdb_j["hash_email"] = hash_email;
                                rocksdb_j["block"] = 1;
                                rocksdb_j["ecdsa_pub_key"] = ecdsa_pub_key_s;
                                rocksdb_j["rsa_pub_key"] = rsa_pub_key;
                                std::string rocksdb_s = rocksdb_j.dump();
                                Poco* poco = new Poco();
                                poco->Put(full_hash_of_new_peer, rocksdb_s);
                                delete poco;
                                std::cout << "zijn we ook hier? " << std::endl;

                                // send latest block to peer
                                nlohmann::json msg;
                                msg["req"] = "new_block";
                                msg["block_nr"] = "1";
                                msg["block"] = block_j;
                                set_resp_msg(msg.dump());
                                std::cout << "Block sent! " << std::endl;
                            }
                            else
                            {
                                delete poco;

                                // communicate intro_peers to chosen_one's with a new_peer req

                                Protocol proto;
                                std::map<std::string, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

                                std::string srv_ip = ""; // only for nat traversal
                                std::string peer_hash = ""; // dunno, still dunno

                                nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
                                message_j["req"] = "new_peer";
                                message_j["email_of_req"] = email_of_req;
                                message_j["prev_hash_of_req"] = prev_hash_req;
                                message_j["full_hash_co"] = my_full_hash;
                                message_j["ecdsa_pub_key"] = ecdsa_pub_key_s;
                                message_j["rsa_pub_key"] = rsa_pub_key;
                                message_j["ip"] = ip_of_peer_;

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

                                Poco* poco = new Poco();
                                Tcp tcp;
                                std::string key, val;
                                for (auto &[key, val] : parts)
                                {
                                    std::cout << key        // string (key)
                                            << ':'  
                                            << val        // string's value
                                            << std::endl;

                                    std::string peer_ip = poco->FindNextPeer(val); // lookup in rocksdb

                                    std::string message = message_j.dump();
                                    tcp.client(srv_ip, peer_ip, peer_hash, message);
                                }
std::cout << "before: " << std::endl;                                   
                                // wait 30 seconds of > 1 MB to create block, to process the timestamp if you are the first new_peer request
                                CreateBlock cb(message_j);
std::cout << "after: " << std::endl;                                   
                                // TODO: rocksdb should be updated when the block is created
                                // so: the new peer should receive the message that the block is created
                                // and then a message should be sent with the rocksdb entries

                                // TODO: CreateBlock isn't final, the hash of the block should point to the chosen_one

                                std::string hash_of_new_block = cb.get_hash_of_new_block();
                                if (hash_of_new_block != "")
                                {
                                    std::string co_for_new_block = poco->FindChosenOne(hash_of_new_block);
                                    if (co_for_new_block == my_full_hash)
                                    {
                                        // I'm the chosen one for creating the block!!!
                                        std::cout << "I'm the chosen_one for block creation!!" << std::endl;
                                    }
                                    else
                                    {
                                        // I'm NOT the chosen one for creating the block!!!
                                        std::cout << "I'm NOT the chosen_one for block creation!!" << std::endl;
                                    }
                                }

                                delete poco;
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

                        std::cout << "verification2p: " << std::endl;
                        std::cout << "ecdsa_p_key: " << "X" << ecdsa_pub_key_s << "X" << std::endl;
                        std::cout << "to_sign_s: " << "X" << to_verify_s << "X" << std::endl;
                        std::cout << "signature: " << "X" << signature << "X" << std::endl;

                        room_.leave(shared_from_this());
                    }
                }
                else
                {
                    delete poco;

                    // user exists already in rocksdb
                    // respond with user_exists

                    std::cout << "!poco->Get(full_hash_req) == \"\" " << std::endl;
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
                // should read the timestamp of the first new_peer request received
                CreateBlock cb(buf_j);

                Auth a;
                std::string my_full_hash = a.get_my_full_hash();

                std::string hash_of_new_block = cb.get_hash_of_new_block();
                if (hash_of_new_block != "")
                {
                    Poco* poco = new Poco();
                    std::string co_for_new_block = poco->FindChosenOne(hash_of_new_block);
                    if (co_for_new_block == my_full_hash)
                    {
                        // I'm the chosen one for creating the block!!!
                        std::cout << "I'm the chosen_one for block creation!!" << std::endl;

                        // tcp.client to all chosen_ones with all new_peers
                        // the hash block and compare hash and verify some other stuff
                        // if ok then save block and tcp.client to all next_layer peers in your bucket
                        // then update rocksdb
                        // then test the whole
                    }
                    else
                    {
                        // I'm NOT the chosen one for creating the block!!!
                        std::cout << "I'm NOT the chosen_one for block creation!!" << std::endl;
                    }
                    delete poco;
                }
            }
        }
    }

    std::vector<std::string> split(const std::string& str, int splitLength)
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

    void set_resp_msg(std::string msg)
    {
        std::vector<std::string> splitted = split(msg, p2p_message::max_body_length);
        for (int i = 0; i < splitted.size(); i++)
        {
            char s[p2p_message::max_body_length + 1];
            strncpy(s, splitted[i].c_str(), sizeof(s));

            resp_msg_.body_length(std::strlen(s));
            std::memcpy(resp_msg_.body(), s, resp_msg_.body_length());
            i == splitted.size() - 1 ? resp_msg_.encode_header(1) : resp_msg_.encode_header(0); // 1 indicates end of message eom, TODO perhaps a set_eom_flag(true) instead of an int

            room_.deliver(resp_msg_);
        }
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
    std::string ip_of_peer_ = socket_.remote_endpoint().address().to_string();
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