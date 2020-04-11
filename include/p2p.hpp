#pragma once

#include <string>
#include <boost/asio.hpp>
#include <iostream>

#include <boost/array.hpp>

using boost::asio::ip::tcp;

using namespace std; 
using namespace boost::asio; 
using namespace boost::asio::ip;

namespace crowd
{
    class p2p_handler
    {
    public:
        void p2p_switch(string, string);
        vector<string> parse_ip_adress_master_peer_json();
    private:
        std::string client(string&, string&);
        int server_main();
        string getDataServer(tcp::socket&);
        void sendDataServer(tcp::socket&, const string&);
        string getDataClient(tcp::socket&);
        void sendDataClient(tcp::socket&, const string&);
        void save_blockchain(string);
    };
}
