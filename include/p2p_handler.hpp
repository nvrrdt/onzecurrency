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

namespace crowd
{
    class p2p_handler
    {
    public:
        void p2p_switch(string);
        vector<string> parse_ip_peers_json();
    private:
        int server_main();
        std::string client(string&, string&);
        string getDataServer(tcp::socket&);
        void sendDataServer(tcp::socket&, const string&);
        string getDataClient(tcp::socket&);
        void sendDataClient(tcp::socket&, const string&);
    };
}

#endif /* P2P_HANDLER_H */