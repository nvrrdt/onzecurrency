#pragma once

#include <string>
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>

#include <boost/array.hpp>

#include "rocksy.hpp"
#include "json.hpp"
#include "prev_hash.hpp"
#include "crypto.hpp"
#include "merkle_tree.hpp"
#include "full_hash.hpp"
#include "auth.hpp"

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
        bool start_crowd(std::map<std::string, std::string> cred);
        void to_json(nlohmann::json& j, const std::vector<std::string>& str) {
            j = nlohmann::json{{"ip_list", str}};
        }
        int ip_string_to_number (const char* pDottedQuad, unsigned int &pIpAddr);
        int number_to_ip_string(uint32 ipAddress, std::string& ip_string);
        static void signal_callback_handler(int signum); // handle ctrl-c
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
        std::string get_last_block_nr();
        std::map<int, std::string> partition_in_buckets(std::string &my_hash, std::string &next_hash);
        std::map<uint32_t, uint256_t>layers_management(uint256_t &amount_of_peers);
        nlohmann::json get_blocks_from(std::string &latest_block_peer);
        std::string get_all_users_from(std::string &latest_block_peer);
        std::string block_plus_one(std::string &block_nr);
        nlohmann::json get_block_at(std::string block_nr_s);
    private:
        int verify_latest_block(std::string &latest_block_peer);
        int communicate_to_all(boost::array<char, 128> &msg);
        std::map<int, std::string> get_calculated_hashes(std::string &my_hash, std::map<uint32_t, uint256_t> &chosen_ones_counter);
    };

    class LookupPeer
    {
    public:
        LookupPeer(std::string &peer_hash)
        {
            key_ = rocksy_->FindCoordinator(peer_hash);
            value_j_ = nlohmann::json::parse(rocksy_->Get(key_));
            id_ = value_j_["id"];
            ip_ = value_j_["ip"];
            server_ = value_j_["server"];
            pub_key_ = value_j_["pub_key"];
        }
        ~LookupPeer()
        {
            delete rocksy_;
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
        Rocksy* rocksy_ = new Rocksy("usersdbreadonly");
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
            key_ = rocksy_->FindServerPeer(peer_hash);
            value_j_ = nlohmann::json::parse(rocksy_->Get(key_));
            id_ = value_j_["id"];
            ip_ = value_j_["ip"];
            server_ = value_j_["server"];
            pub_key_ = value_j_["pub_key"];
        }
        ~LookupPeerIsServer()
        {
            delete rocksy_;
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
        Rocksy* rocksy_ = new Rocksy("usersdbreadonly");
        std::string key_;
        nlohmann::json value_j_;
        std::string ip_;
        std::string id_;
        std::string server_;
        std::string pub_key_;
    };

    
}
