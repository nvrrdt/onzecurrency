#ifndef P2P_HANDLER_H
#define P2P_HANDLER_H

#include <ctime>
#include <string>
#include <boost/asio.hpp>
#include <iostream>

#include <boost/array.hpp>

using boost::asio::ip::tcp;

using namespace std; 
using namespace boost::asio; 
using namespace boost::asio::ip;

extern bool break_server_loop;

namespace crowd
{
    class p2p_handler
    {
    public:
        void p2p_switch(string, string);
        vector<string> parse_ip_peers_json();
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

#endif /* P2P_HANDLER_H */