#pragma once

#include <string>
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>

#include <boost/array.hpp>

#include "poco.hpp"
#include "json.hpp"
#include "prev_hash.hpp"
#include "crypto.hpp"
#include "merkle_tree.hpp"

#include <atomic>
#include <condition_variable>
#include <thread>
#include <chrono>

#include <stack>
#include <memory>

#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision;

using namespace std::chrono_literals;

using namespace std; 
using namespace boost::asio; 
using namespace boost::asio::ip;

namespace unit_test {
struct FooTester;
}

namespace Crowd
{
    class P2p
    {
    public:
        bool start_p2p(std::map<std::string, std::string> cred);
        void to_json(nlohmann::json& j, const std::vector<std::string>& str) {
            j = nlohmann::json{{"ip_list", str}};
        }
    };
    
    class Tcp
    {
    public:
        int server();
        std::string client(std::string &srv_ip, std::string &peer_ip, std::string &peer_hash, std::string &message); // TODO: add a reference & to these strings
        void set_tcp_closed_client(bool &close)
        {
            close_client_ = close;
        }
        bool get_tcp_closed_client()
        {
            return close_client_;
        }
    private:
        bool close_client_;
    };

    class Upnp
    {
    public:
        int Upnp_main();
    private:
        const char * protofix(const char * proto);
        static int SetRedirectAndTest(struct UPNPUrls * urls,
			       struct IGDdatas * data,
			       const char * iaddr,
			       const char * iport,
			       const char * eport,
			       const char * proto,
			       const char * leaseDuration,
			       const char * remoteHost,
			       const char * description,
			       int addAny);
    };

    class Protocol
    {
    public:
        int hello_and_setup(std::string& my_user_login_hash);
        std::string latest_block();
        std::map<std::string, std::string> partition_in_buckets(std::string &my_hash, std::string &next_hash);
        std::map<uint32_t, uint256_t>layers_management(uint256_t &amount_of_peers);
        std::string get_blocks_from(std::string &latest_block_peer);
        void save_blocks_to_blockchain(std::string &msg);
        std::string block_plus_one(std::string &block_nr);
    private:
        int verify_latest_block(std::string &latest_block_peer);
        int communicate_to_all(boost::array<char, 128> &msg);
        std::map<std::string, std::string> get_calculated_hashes(std::string &my_hash, std::map<uint32_t, uint256_t> &chosen_ones_counter);
    };

    class LookupPeer
    {
    public:
        LookupPeer(std::string &peer_hash)
        {
            key_ = poco_.FindChosenOne(peer_hash);
            value_j_ = nlohmann::json::parse(poco_.Get(key_));
            id_ = value_j_["id"];
            ip_ = value_j_["ip"];
            server_ = value_j_["server"];
            pub_key_ = value_j_["pub_key"];
        }
        std::string GetIpPeer()
        {
            return ip_;
        }
        std::string GetIdPeer()
        {
            return id_;
        }
        std::string GetPubKeyPeer()
        {
            return pub_key_;
        }
        bool IsServer() // not behind NAT
        {
            return server_ == "true" ? true : false;
        }
    private:
        Poco poco_;
        std::string key_;
        nlohmann::json value_j_;
        std::string ip_;
        std::string id_;
        std::string server_;
        std::string pub_key_;
    };

    // if Class LookupPeer is behind NAT, is client, then you can call this to find the server in order to do NAT traversal
    class LookupPeerIsServer
    {
    public:
        LookupPeerIsServer(std::string &peer_hash)
        {
            key_ = poco_.FindServerPeer(peer_hash);
            value_j_ = nlohmann::json::parse(poco_.Get(key_));
            id_ = value_j_["id"];
            ip_ = value_j_["ip"];
            server_ = value_j_["server"];
            pub_key_ = value_j_["pub_key"];
        }
        std::string GetIpPeer()
        {
            return ip_;
        }
        std::string GetIdPeer()
        {
            return id_;
        }
        std::string GetPubKeyPeer()
        {
            return pub_key_;
        }
    private:
        Poco poco_;
        std::string key_;
        nlohmann::json value_j_;
        std::string ip_;
        std::string id_;
        std::string server_;
        std::string pub_key_;
    };

    class CreateBlock
    {
    public:
        CreateBlock(nlohmann::json &message_j)
        {
            std::string email_of_req = message_j["email_of_req"];
            Crypto crypto;
            std::string hash_email = crypto.bech32_encode_sha256(email_of_req);
            PrevHash ph;
            std::string prev_hash = ph.get_last_prev_hash_from_blocks();
            std::string hash_email_prev_hash_app = hash_email + prev_hash;
            std::string full_hash_of_new_peer = crypto.bech32_encode_sha256(hash_email_prev_hash_app);
            
            list_of_new_peers_.push_back(full_hash_of_new_peer);

            if (list_of_new_peers_.size() > 2048) // 2048x 512 bit hashes
            {
                std::thread t1(&CreateBlock::signals, this);
                t1.join();
            }
            else
            {
                std::thread t1(&CreateBlock::waits, this, 1, message_j, full_hash_of_new_peer); // TODO: maybe when adding two peers within 30 seconds doesn't work because of this
                t1.join();  // https://en.cppreference.com/w/cpp/thread/condition_variable/wait_until
            }
        }
        std::string get_hash_of_new_block()
        {
            return hash_of_block_;
        }
    private:
        void waits(int idx, nlohmann::json message_j, std::string full_hash_of_new_peer)
        {
            std::unique_lock<std::mutex> lk(cv_m);
            auto now = std::chrono::system_clock::now();
            if(cv.wait_until(lk, now + 30s, [=](){return i == 1;}))
            {
                std::cerr << "Thread " << idx << " finished waiting. i == " << i << '\n';
                // TODO: create block here ...
                // include prev_hash, hash of merkle tree of list_of_new_peers_, the list_of_new_peers_ and calculate but exclude the final hash
                // rocksdb.get(final_hash) to get the chosen_one of chosen_one's

                // TODO: the hash of the block doesn't point to the chosen_one yet, which should be ...
                // now the block is no matter what created after 30 seconds or 1 MB, that's nog right

                nlohmann::json entry_tx_j, entry_transactions_j, exit_tx_j, exit_transactions_j, rocksdb_j;
                                
                merkle_tree mt;
                s_shptr_ = mt.calculate_root_hash(s_shptr_);
                std::string datetime = mt.time_now();
                std::string root_hash_data = s_shptr_->top();
                nlohmann::json block_j = mt.create_block(datetime, root_hash_data, entry_transactions_j_, exit_transactions_j_);
                Protocol proto;
                std::string my_latest_block = proto.latest_block();
                std::string latest_block_plus_one = proto.block_plus_one(my_latest_block);
                std::string block_s = mt.save_block_to_file(block_j, latest_block_plus_one);

                set_hash_of_new_block(block_s);
                
                std::cout << "Block created! " << std::endl;
            }
            else
            {
                std::cerr << "Thread " << idx << " timed out. i == " << i << '\n';

                nlohmann::json to_block_j, entry_tx_j, exit_tx_j;
                
                to_block_j["full_hash"] = full_hash_of_new_peer;
                to_block_j["ecdsa_pub_key"] = message_j["ecdsa_pub_key"];
                to_block_j["rsa_pub_key"] = message_j["rsa_pub_key"];

                s_shptr_->push(to_block_j.dump());

                entry_tx_j["full_hash"] = to_block_j["full_hash"];
                entry_tx_j["ecdsa_pub_key"] = to_block_j["ecdsa_pub_key"];
                entry_tx_j["rsa_pub_key"] = to_block_j["rsa_pub_key"];
                entry_transactions_j_.push_back(entry_tx_j);
                exit_tx_j["full_hash"] = "";
                exit_transactions_j_.push_back(exit_tx_j);
            }
        }
        void signals()
        {
            i = 1;
            std::cerr << "Notifying again...\n";
            cv.notify_all();
        }
        void set_hash_of_new_block(std::string block)
        {
            Crypto crypto;
            hash_of_block_ = crypto.bech32_encode(block);
        }
    private:
        static std::vector<std::string> list_of_new_peers_;
        std::shared_ptr<std::stack<std::string>> s_shptr_ = make_shared<std::stack<std::string>>();
        nlohmann::json entry_transactions_j_;
        nlohmann::json exit_transactions_j_;

        std::string hash_of_block_;

        std::condition_variable cv;
        std::mutex cv_m;
        std::atomic<int> i{0};
    };
}
