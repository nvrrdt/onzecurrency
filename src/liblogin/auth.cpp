#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>

#include "auth.hpp"
#include "configdir.hpp"
#include "poco.hpp"
#include "hash.hpp"
#include "json.hpp"

using namespace Crowd;

std::string Auth::authentication()
{
    std::string network, email, password;

    std::cout << "Network: ";
    std::cin >> network;
    std::cout << "Email adress: ";
    std::cin >> email;
    std::cout << "Password: ";
    std::cin >> password;

    if (!Auth::validateEmail(email))
    {
        return "1";
    }

    std::string string_full_hash = Auth::verifyCredentials(email, password);

    if (Auth::setNetwork(network) == 0 && string_full_hash != "0")
    {
        return string_full_hash;
    }
    else
    {
        return "1";
    }
}

/**
 * setNetwork: Should be implemented later
 */
int Auth::setNetwork(std::string network)
{
    if (network != "Default")
    {
        printf("Network must be 'Default'\n");
        return 1;
    }

    return 0;
}

/**
 * verifyCredentials:
 * if H(H(email)+H(H(password+salt))) is correct
 * then login is ok
 * get email, H(H(email)+H(H(password+salt))) and salt from blockchain in rocksdb
 * if no data from blockchain: create new user
 */
std::string Auth::verifyCredentials(std::string email, std::string string_password)
{
    Hash h;
    std::string email_hashed_from_input;
    h.create_hash(email, email_hashed_from_input);
    
    //updateBlockchain(); !!!!! TODO: important: blockchain must be up-to-date

    Poco p;
    std::string string_poco_response = p.Get(email_hashed_from_input);

    // get email, full_hash and salt from the blockchain
    nlohmann::json json_poco_response = nlohmann::json::parse(string_poco_response);
    
    std::string string_email_hash_from_blockchain = json_poco_response["em"].dump();
    std::string string_full_hash_from_blockchain = json_poco_response["fh"].dump();
    std::string string_salt_from_blockchain = json_poco_response["sa"].dump();

    // create full_hash from the input
    std::string full_hash_and_salt_hashed, full_hash_and_salt_hashed_again, string_full_hash_from_input;
    h.create_hash(string_password + string_salt_from_blockchain, full_hash_and_salt_hashed);
    h.create_hash(full_hash_and_salt_hashed + string_salt_from_blockchain, full_hash_and_salt_hashed_again);
    h.create_hash(email_hashed_from_input + full_hash_and_salt_hashed_again + string_salt_from_blockchain, string_full_hash_from_input);

    if (json_poco_response.is_null()) // TODO: test this for when a key is nonexistant in rocksdb, not sure if this works
    {   
        // TODO: set new salt, set full hash, email: communicate those three to the next_upnp_peer of the full hash
        // put also the full_hash creation procedure in a function
        printf("A new user will be created!\n");
        Auth::createNewUser(email, string_full_hash_from_input);
    }

    // compare input with blockchain
    if (string_full_hash_from_input == string_full_hash_from_blockchain
        && email_hashed_from_input == string_email_hash_from_blockchain)
    {
        return string_full_hash_from_input;
    }
    else
    {
        std::cerr << "ERROR in comparing email and full_hash between input and blockchain!\n";
        return "0";
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

int Auth::createNewUser(std::string email_hashed_from_input, std::string string_full_hash_from_input)
{
    // communicate new user to all

    return 0;
}

uint32_t Auth::changeExistingFullHash(std::string email, std::string string_password)
{
    Random r;
    std::string string_new_salt = std::to_string(r.createSalt());

    Hash h;
    std::string email_hashed_from_input;
    h.create_hash(email, email_hashed_from_input);
    
    // create full_hash from the input
    std::string full_hash_and_salt_hashed, full_hash_and_salt_hashed_again, string_full_hash_from_input;
    h.create_hash(string_password + string_new_salt, full_hash_and_salt_hashed);
    h.create_hash(full_hash_and_salt_hashed, full_hash_and_salt_hashed_again);
    h.create_hash(email_hashed_from_input + full_hash_and_salt_hashed_again, string_full_hash_from_input);

    return 0;
}
