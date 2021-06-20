#include <utility>
#include <sstream>

#include "transactions.hpp"

#include "crypto.hpp"

using namespace Crowd;
using namespace Coin;

void Transactions::add_tx_to_transactions(std::string full_hash_req, std::string to_full_hash, std::string amount)
{
    Crypto crypto;
    std::string string = full_hash_req + to_full_hash + amount; // TODO what if these three are the same in two transactions in the same block?
    std::string tx_hash = crypto.bech32_encode_sha256(string);
    std::vector<std::string> vec1;
    vec1.push_back(full_hash_req);
    vec1.push_back(to_full_hash);
    vec1.push_back(amount);
    std::string dev_amount = calculate_dev_payment(amount);
    vec1.push_back(dev_amount);
    std::pair<std::string, std::shared_ptr<std::vector<std::string>>> pair;
    pair.first = tx_hash;
    pair.second = std::make_shared<std::vector<std::string>>(vec1);
    transactions_.push_back(pair);

    set_latest_transaction(tx_hash, vec1);
}

std::vector<std::pair<std::string, std::shared_ptr<std::vector<std::string>>>> Transactions::get_transactions()
{
    return transactions_;
}

void Transactions::reset_transactions()
{
    transactions_.clear();
}

// void Transactions::remove_tx_from_transactions(std::string tx_hash)
// {
//     transactions_.erase(tx_hash);
// }

void Transactions::set_latest_transaction(std::string tx_hash, std::vector<std::string> vec)
{
    latest_transaction_.clear();
    latest_transaction_[tx_hash] = vec;
}

std::map<std::string, std::vector<std::string>> Transactions::get_latest_transaction()
{
    return latest_transaction_;
}

uint64_t Transactions::calculate_dev_payment_numbers(uint64_t amount)
{
    uint64_t dev_amount = 0.00035 * amount;   // 0.035%/tx is the dev payment
                                                            // TODO maybe introduce a lower limit here
    return dev_amount;
}

std::string Transactions::calculate_dev_payment(std::string amount)
{
    uint64_t amount_number;
    std::istringstream iss(amount);
    iss >> amount_number;

    uint64_t dev_amount_number = 0.00035 * amount_number;   // 0.035%/tx is the dev payment
                                                            // TODO maybe introduce a lower limit here

    std::ostringstream oss;
    oss << dev_amount_number;
    std::string dev_amount = oss.str();

    return dev_amount;
}

std::vector<std::pair<std::string, std::shared_ptr<std::vector<std::string>>>> Transactions::transactions_ = {};
std::map<std::string, std::vector<std::string>> Transactions::latest_transaction_ = {};
