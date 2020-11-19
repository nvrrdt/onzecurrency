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
        bool setNetwork(std::string network);
        std::map<std::string, std::string> verifyCredentials(std::string email, std::string password);
        bool validateEmail(const std::string& email);
        std::map<std::string, std::string> generateFullHash(std::string email, std::string salt, std::string password);
        uint32_t changeExistingFullHash(std::string email, std::string password);
    };

    class Random {
    public:
        Random() = default;
        Random(std::mt19937::result_type seed) : eng(seed) {}

        uint32 createSalt();

    private:
        std::mt19937 eng{std::random_device{}()};
        std::uniform_int_distribution<uint32> uniform_dist{0, UINT32_MAX};
    };
}