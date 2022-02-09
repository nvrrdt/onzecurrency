#include "p2p_network_c.hpp"

#include "full_hash.hpp"
#include "prev_hash.hpp"
#include "rocksy.hpp"
#include "crypto.hpp"
#include "verification_c.hpp"

#include "print_or_log.hpp"

using namespace Coin;

void P2pNetworkC::start_coin()
{
    Common::Print_or_log pl;

    VerificationC verification;
    verification.verify_all_blocks();

    // input to create a transaction (tx)
    while (true)
    {
        std::string to_full_hash = "", amount = "";
        std::cout << "Tx to: ";
        std::cin >> to_full_hash;

        if (to_full_hash == "q")
        {
            // TODO send an intro_offline request here

            P2pNetwork pn;
            pn.set_quit_server_req(true);
            break;
        }

        std::cout << "Amount: ";
        std::cin >> amount;

        std::cout << std::endl;

        if (P2pNetworkC::validate_full_hash(to_full_hash) && P2pNetworkC::validate_amount(amount))
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Send hello_tx"});
            
            // See p2p_network_c.cpp for an explanation (in the beginning of the file)
            FullHash fh;
            std::string my_full_hash = fh.get_full_hash();
            PrevHash ph;
            std::string hash_latest_block = ph.calculate_hash_from_last_block();
            std::string prel_coordinator = my_full_hash + hash_latest_block;

            Rocksy* rocksy = new Rocksy("usersdbreadonly");
            std::string full_hash_coordinator = rocksy->FindChosenOne(prel_coordinator);
            nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(full_hash_coordinator));
            delete rocksy;
            
            std::string ip = contents_j["ip"];
            
            nlohmann::json message_j, to_sign_j;
            message_j["req"] = "hello_tx";
            message_j["full_hash_req"] = my_full_hash;
            message_j["tx_to"] = to_full_hash;
            message_j["amount"] = amount;

            to_sign_j["req"] = message_j["req"];
            to_sign_j["full_hash_req"] = my_full_hash;
            to_sign_j["tx_to"] = to_full_hash;
            to_sign_j["amount"] = amount;
            std::string to_sign_s = to_sign_j.dump();
            // pl.handle_print_or_log({"to_sign_s:", to_sign_s});
            ECDSA<ECP, SHA256>::PrivateKey private_key;
            std::string signature;
            Common::Crypto crypto;
            crypto.ecdsa_load_private_key_from_string(private_key);
            if (crypto.ecdsa_sign_message(private_key, to_sign_s, signature))
            {
                message_j["signature"] = crypto.base64_encode(signature);
            }

            std::string message_s = message_j.dump();

            P2pNetwork pn;
            pn.p2p_client(ip, message_s);
        }
        else
        {
            continue;
        }
    }
}

bool P2pNetworkC::validate_full_hash(std::string to_full_hash)
{
    Common::Crypto crypto;
    bool out_success;
    crypto.bech32_decode(to_full_hash, out_success);

    return out_success;
}

bool P2pNetworkC::validate_amount(std::string amount)
{
    std::string::const_iterator it = amount.begin();
    while (it != amount.end() && std::isdigit(*it)) ++it;
    return !amount.empty() && it == amount.end();
}