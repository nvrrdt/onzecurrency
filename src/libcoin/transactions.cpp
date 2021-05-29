#include "transactions.hpp"

#include "crypto.hpp"

using namespace Crowd;
using namespace Coin;

void Transactions::add_tx_to_transactions(std::string full_hash_req, std::string to_full_hash, std::string amount)
{
    Crypto crypto;
    std::string string = full_hash_req + to_full_hash + amount;
    std::string hash = crypto.bech32_encode_sha256(string);
    std::vector<std::string> vec;
    vec.push_back(full_hash_req);
    vec.push_back(to_full_hash);
    vec.push_back(amount);
    transactions_[hash] = std::make_shared<std::vector<std::string>>(vec);
}

std::map<std::string, std::shared_ptr<std::vector<std::string>>> Transactions::get_transactions()
{
    return transactions_;
}

void Transactions::reset_transactions()
{
    transactions_.clear();
}

std::map<std::string, std::shared_ptr<std::vector<std::string>>> Transactions::transactions_ = {};
