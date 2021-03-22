#include <cstdlib>
#include <deque>
#include <iostream>
#include <sstream>
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
#include "poco.hpp"

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

    void deliver(const p2p_message &msg, p2p_participant_ptr participant)
    {
        recent_msgs_.push_back(msg);
        while (recent_msgs_.size() > max_recent_msgs)
            recent_msgs_.pop_front();

        for (auto p : participants_)
        {
            if (p == participant)
            {
                p->deliver(msg);
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

class MessageVec
{
public:
    static void add_to_message_j_vec(nlohmann::json &message_j)
    {
        message_j_vec_.push_back(message_j);
    }

    static std::vector<nlohmann::json> get_message_j_vec()
    {
        return message_j_vec_;
    }

    static void reset_message_j_vec()
    {
        message_j_vec_.clear();
    }
private:
    static std::vector<nlohmann::json> message_j_vec_;
};

std::vector<nlohmann::json> MessageVec::message_j_vec_ = {};

//----------------------------------------------------------------------

class CreateBlock
{
public:
    CreateBlock(std::vector<nlohmann::json> &message_j_vec)
    {
        merkle_tree mt;

        nlohmann::json m_j, entry_tx_j, entry_transactions_j, exit_tx_j, exit_transactions_j;
        nlohmann::json to_block_j;
        std::string fh_s;

        while (!message_j_vec.empty())
        {
            m_j = message_j_vec.back();
            message_j_vec.pop_back();

            std::string full_hash_req = m_j["full_hash_req"];

            to_block_j["full_hash"] = full_hash_req;
            to_block_j["ecdsa_pub_key"] = m_j["ecdsa_pub_key"];
            to_block_j["rsa_pub_key"] = m_j["rsa_pub_key"];
            s_shptr_->push(to_block_j.dump());

            entry_tx_j["full_hash"] = to_block_j["full_hash"];
            entry_tx_j["ecdsa_pub_key"] = to_block_j["ecdsa_pub_key"];
            entry_tx_j["rsa_pub_key"] = to_block_j["rsa_pub_key"];
            entry_transactions_j.push_back(entry_tx_j);
            exit_tx_j["full_hash"] = "";
            exit_transactions_j.push_back(exit_tx_j);
        }

        s_shptr_ = mt.calculate_root_hash(s_shptr_);
        std::string datetime = mt.time_now();
        std::string root_hash_data = s_shptr_->top();
        nlohmann::json block_j = mt.create_block(datetime, root_hash_data, entry_transactions_j, exit_transactions_j);
        Protocol proto;
        std::string my_latest_block_nr = proto.get_last_block_nr();

        // send hash of this block with the block contents to the co's, forget save_block_to_file
        // is the merkle tree sorted, then find the last blocks that are gathered for all the co's

        // send intro_block to co's
        Poco poco;
        poco.inform_chosen_ones(my_latest_block_nr, block_j);

        //std::string block_s = mt.save_block_to_file(block_j, my_latest_block_nr);
std::cout << "--------5: " << std::endl;
        //set_hash_of_new_block(block_s);
    }

    std::string get_hash_of_new_block()
    {
        return hash_of_block_;
    }
private:
    void set_hash_of_new_block(std::string block)
    {
        Crypto crypto;
        hash_of_block_ = crypto.bech32_encode(block);
    }
private:
    std::shared_ptr<std::stack<std::string>> s_shptr_ = make_shared<std::stack<std::string>>();
    std::string hash_of_block_;
};

//----------------------------------------------------------------------

class p2p_session
    : public p2p_participant,
      public std::enable_shared_from_this<p2p_session>
{
public:
    p2p_session(tcp::socket socket, p2p_room &room, MessageVec message_j_vec)
        : socket_(std::move(socket)),
          room_(room),
          message_j_vec_(message_j_vec)
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
                                    std::cout << "ec server: " << ec << std::endl;
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
                // std::string prev_hash_req = buf_j["prev_hash_of_req"];
                std::string ecdsa_pub_key_s = buf_j["ecdsa_pub_key"];
                std::string rsa_pub_key = buf_j["rsa_pub_key"];
                //std::cout << "1p: " << buf_j.dump() << std::endl;
                std::string signature = buf_j["signature"];
                //std::cout << "2p: " << std::endl;
                std::string req_latest_block = buf_j["latest_block"];
                std::string req_ip = buf_j["ip"];

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

                            room_.join(shared_from_this());
                            
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
                                    set_resp_msg(msg.dump());
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

                                    std::string u = user.dump();
                                    std::string value = rocksy->Get(u);
                                    msg["value"] = value;

                                    set_resp_msg(msg.dump());
                                }
                                delete rocksy;
                            }
                            else if (req_latest_block > my_latest_block)
                            {
                                // TODO: update your own blockchain
                                nlohmann::json msg;
                                msg["req"] = "update_my_blocks_and_rocksdb";
                                msg["block_nr"] = my_latest_block;
                                set_resp_msg(msg.dump());
                            }

                            std::cout << "1 or more totalamountofpeers! " << std::endl;

                            // communicate intro_peers to chosen_one's with a new_peer req

                            std::map<std::string, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

                            std::string srv_ip = ""; // only for nat traversal
                            std::string peer_hash = ""; // dunno, still dunno

                            nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
                            message_j["req"] = "new_peer";
                            // message_j["email_of_req"] = email_of_req; // new_peers don't need to know this
                            email_prev_hash_concatenated = email_of_req + real_prev_hash_req;
                            full_hash_req =  crypto->bech32_encode_sha256(email_prev_hash_concatenated);
                            message_j["full_hash_req"] = full_hash_req; // refreshed full_hash_req
                            message_j["prev_hash_of_req"] = real_prev_hash_req;
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

                            Tcp tcp;
                            std::string key, val;
                            for (auto &[key, val] : parts)
                            {
                                // std::cout << "key: " << key << ", val: " << val << std::endl;
                                
                                Rocksy* rocksy = new Rocksy();

                                std::string peer_id = rocksy->FindChosenOne(val); // lookup in rocksdb
                                nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(peer_id));
                                std::string peer_ip = value_j["ip"];

                                delete rocksy;

                                // if peer ip == this server's ip --> send new_peer to kids
                                // --> from_to(my_hash, my_hash) if just me then connected_peers + from_to(my_hash, next hash)
                                // --> if more then t.client to same layer co
                                // in bucket --> createblock --> coord connects to all co --> co connect to other co --> communicate final_hash --> register amount of ok's and nok's

                                // inform the underlying network
                                if (req_ip == peer_ip)
                                {
                                    // inform server's underlying network
                                    std::cout << "Inform my underlying network as co" << std::endl;

                                    // message to connected peer_
                                    set_resp_msg(message_j.dump());

                                    std::map<std::string, std::string> parts_underlying = proto.partition_in_buckets(my_full_hash, parts["2"]);
                                    // set_resp_msg_() + from_to(my_hash, next_hash) {if ip_of_peer_ == ip_from_to(my_hash, next_hash)} --> new_peer
                                    std::string key2, val2;
                                    for (auto &[key2, val2] : parts_underlying)
                                    {
                                        Rocksy* rocksy = new Rocksy();

                                        std::string peer_id = rocksy->FindChosenOne(val2); // lookup in rocksdb
                                        nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(peer_id));
                                        std::string underlying_peer_ip = value_j["ip"];

                                        delete rocksy;

                                        std::cout << "Non-connected underlying peers - client" << std::endl;
                                        // message to non-connected peers
                                        std::string message = message_j.dump();
                                        tcp.client(srv_ip, underlying_peer_ip, peer_hash, message);
                                    }
                                }
                                else
                                {
                                    // inform the other peer's in the same layer (as coordinator)
                                    std::cout << "Inform my equal layer as coordinator" << std::endl;

                                    std::string message = message_j.dump();
                                    tcp.client(srv_ip, peer_ip, peer_hash, message);
                                }
                            }

                            // Update rocksdb
                            message_j["rocksdb"]["prev_hash"] = real_prev_hash_req;
                            message_j["rocksdb"]["full_hash"] = full_hash_req;

                            // wait 20 seconds of > 1 MB to create block, to process the timestamp if you are the first new_peer request
                            message_j_vec_.add_to_message_j_vec(message_j);
                            
                            if (message_j_vec_.get_message_j_vec().size() > 2048) // 2048x 512 bit hashes
                            {
                                // Create block
                                std::vector<nlohmann::json> m_j_v = message_j_vec_.get_message_j_vec();
                                CreateBlock cb(m_j_v);
                            }
                            else if (message_j_vec_.get_message_j_vec().size() == 1)
                            {
                                // wait 20 secs
                                // then create block
                                // if root_hash == me as coordinator ... connect to all co's
                                // ... see below at new_peer

                                std::thread t(&p2p_session::get_sleep_and_create_block, this);
                                t.detach();
                            }

                            // TODO intro_peer is doesn't know its full_hash until new_block is send
                            // but the other users don't know the whereabouts of intro_peer
                            // wait for flag when a new_block is communicated, then communicate its full_hash to the new peer
                            // wait for flag
                            // room_.deliver(full_hash) --> client should save full_hash
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
                            std::string peer_ip = value_j["ip"];
                            delete rocksy;

                            message_j["ip_co"] = peer_ip;
                            set_resp_msg(message_j.dump());
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

                room_.leave(shared_from_this());

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
                
                if (message_j_vec_.get_message_j_vec().size() > 2048) // 2048x 512 bit hashes
                {
                    // Create block
                    std::vector<nlohmann::json> m_j_v = message_j_vec_.get_message_j_vec();
                    CreateBlock cb(m_j_v);
                }
                else if (message_j_vec_.get_message_j_vec().size() == 1)
                {
                    // wait 20 secs
                    // then create block --> don't forget the counter in the search for a coordinator
                    // if root_hash == me as coordinator ... connect to all co's
                    std::thread t(&p2p_session::get_sleep_and_create_block, this);
                    t.detach();
                }

                room_.leave(shared_from_this());
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
            }
            else if (buf_j["req"] == "new_block")
            {
                // new_block
                std::cout << "New_block: " << std::endl;
            }
        }
    }

    void get_sleep_and_create_block()
    {
        std::this_thread::sleep_for(std::chrono::seconds(20));

        std::cout << "message_j_vec.size() in CreateBlock: " << message_j_vec_.get_message_j_vec().size() << std::endl;

        std::vector<nlohmann::json> m_j_v = message_j_vec_.get_message_j_vec();
        CreateBlock cb(m_j_v);
        std::cout << "Block created!!" << std::endl;
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

            room_.deliver(resp_msg_, shared_from_this());
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

    MessageVec message_j_vec_;

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
                    std::make_shared<p2p_session>(std::move(socket), room_, mv_)->start();
                }

                do_accept();
            });
    }

    tcp::acceptor acceptor_;
    p2p_room room_;

    MessageVec mv_;
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