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
        void save_full_hash_to_file(std::string& full_hash);
        std::string get_full_hash_from_file();
    };
    
    class Tcp
    {
    public:
        int server();
        std::string client(std::string &srv_ip, std::string &peer_ip, std::string &peer_hash, std::string &message); // TODO: add a reference & to these strings
        void set_tcp_closed_client(std::string closed)
        {
            closed_client_ = closed;
        }
        std::string get_tcp_closed_client()
        {
            return closed_client_;
        }
        void set_ip_new_co(std::string &ip)
        {
            ip_new_co_ = ip;
        }
        std::string get_ip_new_co()
        {
            return ip_new_co_;
        }
    private:
        std::string closed_client_;
        std::string ip_new_co_;
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
        std::string get_last_block_nr();
        std::map<int, std::string> partition_in_buckets(std::string &my_hash, std::string &next_hash);
        std::map<uint32_t, uint256_t>layers_management(uint256_t &amount_of_peers);
        std::string get_blocks_from(std::string &latest_block_peer);
        std::string get_all_users_from(std::string &latest_block_peer);
        //void save_blocks_to_blockchain(std::string &msg);
        std::string block_plus_one(std::string &block_nr);
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
            key_ = rocksy_.FindChosenOne(peer_hash);
            value_j_ = nlohmann::json::parse(rocksy_.Get(key_));
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
        Rocksy rocksy_;
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
            key_ = rocksy_.FindServerPeer(peer_hash);
            value_j_ = nlohmann::json::parse(rocksy_.Get(key_));
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
        Rocksy rocksy_;
        std::string key_;
        nlohmann::json value_j_;
        std::string ip_;
        std::string id_;
        std::string server_;
        std::string pub_key_;
    };

    
}
