#include <iostream>
#include <string>
#include <vector>

#include "auth.hpp"

using namespace crowd;

std::vector<std::string> auth::authentication()
{
    std::string network, email, password;

    std::cout << "Network: ";
    std::cin >> network;
    std::cout << "Email adress: ";
    std::cin >> email;
    std::cout << "Password: ";
    std::cin >> password;

    std::vector<std::string> cred;
    cred.push_back(network);
    cred.push_back(email);
    cred.push_back(password);

    return cred;
}