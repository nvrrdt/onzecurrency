#pragma once

#include <string>
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>

#include <boost/array.hpp>

#include "poco.hpp"
#include "json.hpp"

#include <atomic>
#include <condition_variable>
#include <thread>
#include <chrono>

#include <stack>
#include <memory>

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
        std::string client(std::string &srv_ip, std::string &peer_ip, std::string &peer_hash, std::string &message, std::string &pub_key); // TODO: add a reference & to these strings
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
        std::map<std::string, uint32_t> layer_management(std::string &total_amount_of_peers);
    private:
        int verify_latest_block(std::string &latest_block_peer);
        int communicate_to_all(boost::array<char, 128> &msg);
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
        CreateBlock(std::string &email_of_req, std::string &hash_of_req)
        {
            list_of_new_peers_.push_back(hash_of_req);

            if (list_of_new_peers_.size() > 2048) // 2048x 512 bit hashes
            {
                std::thread t1(&CreateBlock::signals, this);
                t1.join();
            }
            else
            {
                std::thread t1(&CreateBlock::waits, this, 1); // TODO: maybe when adding two peers within 30 seconds doesn't work because of this
                t1.join();                // https://en.cppreference.com/w/cpp/thread/condition_variable/wait_until
            }
        }
    private:
        void waits(int idx)
        {
            std::unique_lock<std::mutex> lk(cv_m);
            auto now = std::chrono::system_clock::now();
            if(cv.wait_until(lk, now + 30s, [=](){return i == 1;}))
                std::cerr << "Thread " << idx << " finished waiting. i == " << i << '\n';
                // TODO: create block here ...
                // include prev_hash, hash of merkle tree of list_of_new_peers_, the list_of_new_peers_ and calculate but exclude the final hash
                // rocksdb.get(final_hash) to get the chosen_one of chosen_one's
            else
                std::cerr << "Thread " << idx << " timed out. i == " << i << '\n';
                // TODO: create block here ...
        }
        void signals()
        {
            // std::this_thread::sleep_for(10ms);
            // std::cerr << "Notifying...\n";
            // cv.notify_all();
            // std::this_thread::sleep_for(10ms);
            i = 1;
            std::cerr << "Notifying again...\n";
            cv.notify_all();
        }
    private:
        static std::vector<std::string> list_of_new_peers_;

        std::condition_variable cv;
        std::mutex cv_m;
        std::atomic<int> i{0};
    };
}
