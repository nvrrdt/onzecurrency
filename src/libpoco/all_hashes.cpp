#include "all_hashes.hpp"

using namespace Poco;

void AllHashes::add_to_all_hashes(enet_uint32 participant, std::string hash_of_email)
{
    std::vector<std::string> vec;
    vec.push_back(hash_of_email);
    all_hashes_[participant] = std::make_shared<std::vector<std::string>>(vec);
}

std::map<enet_uint32, std::shared_ptr<std::vector<std::string>>> AllHashes::get_all_hashes()
{
    return all_hashes_;
}

void AllHashes::reset_all_hashes()
{
    all_hashes_.clear();
}

std::map<enet_uint32, std::shared_ptr<std::vector<std::string>>> AllHashes::all_hashes_ = {};