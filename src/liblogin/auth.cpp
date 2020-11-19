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

using namespace Crowd;

std::map<std::string, std::string> Auth::authentication()
{
    std::string network, email, password;

    std::cout << "Network: ";
    std::cin >> network;
    std::cout << "Email adress: ";
    std::cin >> email;
    std::cout << "Password: ";
    std::cin >> password;

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
        printf("Network must be 'Default'\n");
        return true;
    }

    return false;
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
    Poco p;
    std::string string_poco_response = p.Get(email_hashed_from_input);

    // get email, full_hash and salt from the blockchain
    nlohmann::json json_poco_response = nlohmann::json::parse(string_poco_response);

    if (json_poco_response.is_null()) // TODO: test this for when a key is nonexistant in rocksdb, not sure if this works
    {   
        printf("A new user will be created!\n");

        Random r;
        uint32 salt = r.createSalt();
        std::string strsalt = std::to_string(salt);

        std::map<std::string, std::string> cred = Auth::generateFullHash(email, strsalt, password);
        cred["new_peer"] = "true";
        cred["error"] = "false";

        return cred;
    }
    else
    {
        // get data from rocksdb
        std::string string_email_hash_from_blockchain = json_poco_response["em"].dump();
        std::string string_full_hash_from_blockchain = json_poco_response["fh"].dump();
        std::string string_salt_from_blockchain = json_poco_response["sa"].dump();

        // get data from input
        std::map<std::string, std::string> cred = Auth::generateFullHash(email, string_salt_from_blockchain, password);

        // compare input with blockchain
        if (cred["full_hash"] == string_full_hash_from_blockchain
            && cred["email_hashed"] == string_email_hash_from_blockchain)
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

uint32 Random::createSalt()
{
    uint32 salt = uniform_dist(eng);

    return salt;
}

std::map<std::string, std::string> Auth::generateFullHash(std::string email, std::string salt, std::string password)
{
    Hash h;
    std::string hps, h_hps_s, he, full_hash;
    h.create_hash(password + salt, hps);
    h.create_hash(hps + salt, h_hps_s);
    h.create_hash(email, he);
    h.create_hash(he + h_hps_s + salt, full_hash);

    std::map<std::string, std::string> cred;
    cred["email"] = email;
    cred["email_hashed"] = he;
    cred["salt"] = salt;
    cred["full_hash"] = full_hash;

    return cred;
}

uint32_t Auth::changeExistingFullHash(std::string email, std::string password)
{
    Random r;
    std::string string_new_salt = std::to_string(r.createSalt());

    Hash h;
    std::string email_hashed_from_input;
    h.create_hash(email, email_hashed_from_input);
    
    // create full_hash from the input !!! above is a better example for this hashing !!!
    std::string full_hash_and_salt_hashed, full_hash_and_salt_hashed_again, string_full_hash_from_input;
    h.create_hash(password + string_new_salt, full_hash_and_salt_hashed);
    h.create_hash(full_hash_and_salt_hashed, full_hash_and_salt_hashed_again);
    h.create_hash(email_hashed_from_input + full_hash_and_salt_hashed_again, string_full_hash_from_input);

    // TODO: doesn't work yet!!!!!
    return 0;
}
