#ifndef __PING_HPP_
#define __PING_HPP_

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <cerrno>
#include <cstring>

namespace crowd
{
    class system_ping
    {
        public:
            int test_connection (std::string ip_address, int max_attempts, bool check_eth_port = false, int eth_port_number = 0);
        private:
            int ping_ip_address(std::string ip_address, int max_attempts, std::string& details);
    };
}

#endif