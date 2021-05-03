#include "all_full_hashes.hpp"

using namespace Crowd;

void AllFullHashes::add_to_all_full_hashes(enet_uint32 participant, std::string full_hash_req)
{
    all_full_hashes_[participant] = std::make_shared<std::string>(full_hash_req);
}

std::map<enet_uint32, std::shared_ptr<std::string>> AllFullHashes::get_all_full_hashes()
{
    return all_full_hashes_;
}

void AllFullHashes::reset_all_full_hashes()
{
    all_full_hashes_.clear();
}

std::map<enet_uint32, std::shared_ptr<std::string>> AllFullHashes::all_full_hashes_ = {};