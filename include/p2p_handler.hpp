#ifndef P2P_HANDLER_H
#define P2P_HANDLER_H

#include <boost/asio.hpp>
#include <iostream>

using namespace std; 
using namespace boost::asio; 
using namespace boost::asio::ip;

namespace crowd
{
    class p2p_handler
    {
    public:
        void p2p_switch();
    private:
        string getData(tcp::socket& socket);
        void sendData(tcp::socket& socket, const string& message);
    };
}

#endif /* P2P_HANDLER_H */