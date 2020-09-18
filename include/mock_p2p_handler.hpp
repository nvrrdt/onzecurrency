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
        p2p_handler() {};
        virtual ~p2p_handler() {}

        virtual void p2p_switch(string, string) {};
        virtual vector<string> parse_ip_peers_json() {};
        virtual std::string client(string&, string&) {};
        virtual int server_main() {};
        virtual string getDataServer(tcp::socket&) {};
        virtual void sendDataServer(tcp::socket&, const string&) {};
        virtual string getDataClient(tcp::socket&) {};
        virtual void sendDataClient(tcp::socket&, const string&) {};
        virtual void save_blockchain(string) {};
    };
}

#endif /* P2P_HANDLER_H */