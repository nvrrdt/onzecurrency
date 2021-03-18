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
#include "crypto.hpp"
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
    Crypto crypto;
    PrevHash ph;
    std::string hash_email = crypto.bech32_encode_sha256(email);
    std::string prev_hash = ph.get_my_prev_hash_from_file();
    std::string my_full_hash_s = hash_email + prev_hash;
    my_full_hash_ =  crypto.bech32_encode_sha256(my_full_hash_s);
    Poco* p = new Poco();
    std::string database_response = p->Get(my_full_hash_);
    delete p;

    std::map<std::string, std::string> cred;
    std::string private_key;
    crypto.ecdsa_load_private_key_as_string(private_key);
    if (private_key == "" && prev_hash == "")
    {
        // new user is created
        printf("A new user will be created!\n");

        cred["email"] = email;
        cred["email_hashed"] = hash_email;

        // generate a new keypair for the signature
        crypto.ecdsa_generate_and_save_keypair();
        std::string ecdsa_pub_key;
        crypto.ecdsa_load_public_key_as_string(ecdsa_pub_key);

        // generate a new keypair for rsa
        crypto.rsa_generate_and_save_keypair();
        std::string rsa_pub_key;
        crypto.rsa_load_public_key_as_string_from_file(rsa_pub_key);

        cred["ecdsa_pub_key"] = ecdsa_pub_key;
        cred["rsa_pub_key"] = rsa_pub_key;
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

        std::string ecdsa_pub_key_from_file;
        crypto.ecdsa_load_public_key_as_string(ecdsa_pub_key_from_file);
        cred["ecdsa_pub_key"] = ecdsa_pub_key_from_file;

        std::string rsa_pub_key_from_file;
        crypto.rsa_load_public_key_as_string_from_file(rsa_pub_key_from_file);
        cred["rsa_pub_key"] = rsa_pub_key_from_file;

        nlohmann::json json_response = nlohmann::json::parse(database_response);

        // get data from rocksdb
        std::string ecdsa_pub_key_from_blockchain = json_response["ecdsa_pub_key"];

        // compare ecdsa_pub_key with blockchain
        if (ecdsa_pub_key_from_file == ecdsa_pub_key_from_blockchain)
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