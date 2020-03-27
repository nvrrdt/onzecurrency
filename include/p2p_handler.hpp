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
        void p2p_switch();
        vector<string> parse_ip_peers_json();
    private:
        int server_main();
        int client(string&);
    };
}

#endif /* P2P_HANDLER_H */