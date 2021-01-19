#pragma once

#include <string>
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>

#include <boost/array.hpp>

#include "poco.hpp"
#include "json.hpp"

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
        bool start_p2p(std::string my_id);
        void to_json(nlohmann::json& j, const std::vector<std::string>& str) {
            j = nlohmann::json{{"ip_list", str}};
        }
    };
    
    class Tcp
    {
    public:
        int server();
        std::string client(std::string srv_ip, std::string peer_ip, std::string peer_hash, std::string message, std::string pub_key); // TODO: add a reference & to these strings
        bool set_close_client(bool close);
        std::string remove_trailing_characters(std::string buf);
    private:
        bool close_client_ = false;
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
        std::map<std::string, uint32_t> layer_management(std::string total_amount_of_peers);
    private:
        int verify_latest_block(std::string latest_block_peer);
        int communicate_to_all(boost::array<char, 128> msg);
    };

    class LookupPeer
    {
    public:
        LookupPeer(std::string peer_hash)
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
        LookupPeerIsServer(std::string peer_hash)
        {
            key_ = poco_.FindUpnpPeer(peer_hash);
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
}

extern "C"
{
    int ParseIPv4OrIPv6 ( const char** ppszText, unsigned char* abyAddr, int* pnPort, int* pbIsIPv6 );
}