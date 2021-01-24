#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <map>

#include "auth.hpp"
#include "configdir.hpp"
#include "poco.hpp"
#include "hash.hpp"
#include "json.hpp"
#include "crypto.hpp"

using namespace Crowd;

std::map<std::string, std::string> Auth::authentication()
{
    std::string network, email, password;

    std::cout << "Network [Default]: ";
    std::getline(std::cin, network);
    // std::cin >> network;
    std::cout << "Email adress: ";
    std::cin >> email;
    std::cout << "Password: ";
    std::cin >> password;

    if (network == "") network = "Default";

    std::map<std::string, std::string> cred = Auth::verifyCredentials(email, password);

    if (Auth::validateEmail(email) == true && Auth::setNetwork(network) == true && cred["error"] == "false")
    {
        return cred;
    }
    else
    {
        cred.clear();
        cred["error"] = "true";

        return cred;
    }
}

/**
 * setNetwork: Should be implemented later
 */
bool Auth::setNetwork(std::string network)
{
    if (network == "Default")
    {
        return true;
    }
    else
    {
        printf("Network must be 'Default'\n");
        return false;
    }
}

/**
 * verifyCredentials:
 * if H(H(email)+H(H(password+salt)+salt)+salt) is correct
 * then login is ok
 * if no data from blockchain: create new user
 */
std::map<std::string, std::string> Auth::verifyCredentials(std::string email, std::string password)
{
    //updateBlockchain(); !!!!! TODO: important: blockchain must be up-to-date

    Hash h;
    std::string email_hashed_from_input;
    h.create_hash(email, email_hashed_from_input);
    std::cout << "email hash: " << email_hashed_from_input << std::endl;
    Poco p;
    std::string string_poco_response = p.Get(email_hashed_from_input);

    std::map<std::string, std::string> cred;
    cred["email"] = email;
    cred["email_hashed"] = email_hashed_from_input;

    if (string_poco_response == "")
    {   
        printf("A new user will be created!\n");

        // generate a new keypair for the signature
        Crypto c;
        c.generate_and_save_keypair();
        auto pub_key = c.get_pub_key();

        cred["pub_key"] = pub_key;

        cred["new_peer"] = "true";
        cred["error"] = "false";

        return cred;
    }
    else
    {
        // user is existant:
        Crypto c;
        auto pub_key = c.get_pub_key();

        cred["pub_key"] = pub_key;

        nlohmann::json json_poco_response = nlohmann::json::parse(string_poco_response);

        // get data from rocksdb
        std::string pub_key_from_blockchain = json_poco_response["pub_key"].dump();

        // compare pub_key with blockchain
        if (pub_key == pub_key_from_blockchain)
        {
            cred["new_peer"] = "false";
            cred["error"] = "false";

            return cred;
        }
        else
        {
            std::cerr << "ERROR in comparing email and full_hash between input and blockchain!\n";

            cred["error"] = "true";
            return cred;
        }
    }
}

bool Auth::validateEmail(const std::string& email)
{
   // define a regular expression
   const std::regex pattern
      ("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");

   // try to match the string with the regular expression
   return std::regex_match(email, pattern);
}
