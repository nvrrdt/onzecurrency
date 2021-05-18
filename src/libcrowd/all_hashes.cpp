#include "all_hashes.hpp"

using namespace Crowd;

void AllHashes::add_to_all_hashes(enet_uint32 participant, std::string full_hash_req, std::string prev_hash_req)
{
    std::vector<std::string> vec;
    vec.push_back(full_hash_req);
    vec.push_back(prev_hash_req);
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