#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <random>

typedef unsigned int uint32;

namespace Crowd
{
    class Auth
    {
    public:
        std::map<std::string, std::string> authentication();
        bool setNetwork(std::string &network);
        std::map<std::string, std::string> verifyCredentials(std::string &email, std::string &password);
        bool validateEmail(const std::string &email);
        std::string get_my_full_hash();
    private:
        static std::string my_full_hash_;
    };
}