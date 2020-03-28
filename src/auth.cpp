#include <iostream>
#include <string>

#include "auth.hpp"
#include "merkle_tree.hpp"

using namespace crowd;

void auth::authentication()
{
    std::string network, email, password;

    std::cout << "Network: ";
    std::cin >> network;
    std::cout << "Email adress: ";
    std::cin >> email;
    std::cout << "Password: ";
    std::cin >> password;

    merkle_tree mt;
    mt.create_user(email, password); // TODO: authenticate (look in blockchain) OR create_user
}