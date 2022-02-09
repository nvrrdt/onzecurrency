#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <map>

#include "auth.hpp"
#include "configdir.hpp"
#include "rocksy.hpp"
#include "json.hpp"
#include "crypto.hpp"
#include "prev_hash.hpp"
#include "full_hash.hpp"

#include "print_or_log.hpp"

using namespace Crowd;

std::map<std::string, std::string> Auth::authentication(std::string network, std::string email)
{
    if (network == "") network = "Default";
    
    std::map<std::string, std::string> cred = Auth::verifyCredentials(email);

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
    Common::Print_or_log pl;

    if (network == "Default")
    {
        return true;
    }
    else
    {
        pl.handle_print_or_log({"Network must be 'Default'"});
        return false;
    }
}

/**
 * verifyCredentials:
 */
std::map<std::string, std::string> Auth::verifyCredentials(std::string &email)
{
    Common::Print_or_log pl;

    Common::Crypto crypto;
    std::string hash_email = crypto.bech32_encode_sha256(email);
    PrevHash ph;
    std::string prev_hash = ph.get_my_prev_hash_from_file();
    FullHash fh;
    std::string full_hash =  fh.get_full_hash();
    Rocksy* rocksy = new Rocksy("usersdb");
    std::string database_response = rocksy->Get(full_hash);
    delete rocksy;

    std::map<std::string, std::string> cred;
    std::string private_key;
    crypto.ecdsa_load_private_key_as_string(private_key);
    if (private_key == "" && prev_hash == "" && full_hash == "")
    {
        // new user is created
        pl.handle_print_or_log({"A new user will be created!"});

        cred["email"] = email;
        cred["email_hashed"] = hash_email;
        cred["prev_hash"] = prev_hash;
        cred["full_hash"] = full_hash;

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
        pl.handle_print_or_log({"The user exists!"});
        cred["email"] = email;
        cred["email_hashed"] = hash_email;
        cred["prev_hash"] = prev_hash;
        cred["full_hash"] = full_hash;

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
            Common::Print_or_log pl;
            pl.handle_print_or_log({"ERROR in comparing email and full_hash between input and blockchain!"});

            cred["error"] = "true";
            return cred;
        }
    }
    else
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"User not in database, priv_key and prev_key present!"}); // TODO: multiple persons should be able to login

        Rocksy* rocksy = new Rocksy("usersdb");
        rocksy->DatabaseDump();
        delete rocksy;

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
