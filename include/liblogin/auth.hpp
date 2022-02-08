#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

typedef unsigned int uint32;

namespace Crowd
{
    class Auth
    {
    public:
        std::map<std::string, std::string> authentication(std::string network, std::string email);
        bool setNetwork(std::string &network);
        std::map<std::string, std::string> verifyCredentials(std::string &email);
        bool validateEmail(const std::string &email);
    private:
    };
}