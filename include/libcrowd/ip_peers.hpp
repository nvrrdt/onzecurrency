#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include "configdir.hpp"
#include "json.hpp"

namespace Crowd
{
    class IpPeers
    {
    public:
        IpPeers();
        std::vector<std::string> get_ip_s()
        {
            return ip_s_;
        }
    private:
        std::vector<std::string> ip_s_;
    };
}