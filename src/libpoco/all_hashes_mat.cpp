#include "all_hashes_mat.hpp"

using namespace Poco;

void IpHEmail::add_to_ip_hemail_vec(enet_uint32 participant, std::string hash_of_email)
{
    std::vector<std::string> vec;
    vec.push_back(hash_of_email);
    ip_hemail_vec_[participant] = std::make_shared<std::vector<std::string>>(vec);
}

std::map<enet_uint32, std::shared_ptr<std::vector<std::string>>> IpHEmail::get_all_ip_hemail_vec()
{
    return ip_hemail_vec_;
}

void IpHEmail::reset_ip_hemail_vec()
{
    ip_hemail_vec_.clear();
}

void IpAllHashes::add_ip_all_hashes_to_ip_all_hashes_vec(nlohmann::json &message_j)
{
    //
}

void IpAllHashes::add_ip_all_hashes_vec_to_ip_all_hashes_mat()
{
    //
}

std::vector<std::vector<std::shared_ptr<nlohmann::json>>> IpAllHashes::get_ip_all_hashes_mat()
{
    return ip_all_hashes_mat_;
}

void IpAllHashes::reset_ip_all_hashes_mat()
{
    //
}

std::map<enet_uint32, std::shared_ptr<std::vector<std::string>>> IpHEmail::ip_hemail_vec_ = {};

std::vector<std::shared_ptr<nlohmann::json>> IpAllHashes::ip_all_hashes_vec_ = {};
std::vector<std::vector<std::shared_ptr<nlohmann::json>>> IpAllHashes::ip_all_hashes_mat_ = {};