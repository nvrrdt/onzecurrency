#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <map>

#include "auth.hpp"
#include "configdir.hpp"
#include "poco.hpp"
#include "json.hpp"
#include "crypto_shab58.hpp"
#include "crypto_ecdsa.hpp"
#include "prev_hash.hpp"

using namespace Crowd;

std::map<std::string, std::string> Auth::authentication()
{
    std::string network = "Default", email = "", password = "";

    // std::cout << "Network [Default]: ";
    // std::getline(std::cin, network);
    // std::cin >> network;
    std::cout << "Email adress: ";
    std::cin >> email;
    // std::cout << "Password: ";
    // std::cin >> password;

    // if (network == "") network = "Default";

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
bool Auth::setNetwork(std::string &network)
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
 */
std::map<std::string, std::string> Auth::verifyCredentials(std::string &email, std::string &password)
{
    Shab58 s;
    Ecdsa e;
    PrevHash ph;
    std::string hash_email = s.create_base58_hash(email);
    std::string prev_hash = ph.get_prev_hash_from_file();
    my_full_hash_ =  s.create_base58_hash(hash_email + prev_hash);
    Poco p;
    std::string database_response = p.Get(my_full_hash_);

    std::map<std::string, std::string> cred;
    if (e.get_priv_key() == "" && prev_hash == "")
    {
        // new user is created
        printf("A new user will be created!\n");

        cred["email"] = email;
        cred["email_hashed"] = hash_email;

        // generate a new keypair for the signature
        e.generate_and_save_keypair();
        auto pub_key = e.get_pub_key();

        cred["pub_key"] = pub_key;
        cred["new_peer"] = "true";
        cred["error"] = "false";

        return cred;
    }
    else if (database_response != "")
    {
        // user is existant:
        printf("The user exists!\n");
        cred["email"] = email;
        cred["email_hashed"] = hash_email;
        cred["prev_hash"] = prev_hash;
        cred["full_hash"] = my_full_hash_;

        std::string pub_key_from_file = e.get_pub_key();
        cred["pub_key"] = pub_key_from_file;

        nlohmann::json json_response = nlohmann::json::parse(database_response);

        // get data from rocksdb
        std::string pub_key_from_blockchain = json_response["pub_key"];

        // compare pub_key with blockchain
        if (pub_key_from_file == pub_key_from_blockchain)
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
    else
    {
        std::cerr << "User not in database, priv_key and prev_key present!\n"; // TODO: multiple persons should be able to login

        cred["error"] = "true";
        return cred;
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

std::string Auth::get_my_full_hash()
{
    return my_full_hash_;
}