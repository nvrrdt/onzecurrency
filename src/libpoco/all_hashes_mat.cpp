#include "all_hashes_mat.hpp"

using namespace Poco;

void IpHEmail::add_ip_hemail_to_ip_hemail_vec(std::string participant, std::string hash_of_email)
{
    std::pair<std::string, std::string> myPair = std::make_pair(participant, hash_of_email);
    std::shared_ptr<std::pair<std::string, std::string>> ptr(new std::pair<std::string, std::string> (myPair));
    ip_hemail_vec_.push_back(ptr);
}

std::vector<std::shared_ptr<std::pair<std::string, std::string>>> IpHEmail::get_all_ip_hemail_vec()
{
    return ip_hemail_vec_;
}

void IpHEmail::reset_ip_hemail_vec()
{
    ip_hemail_vec_.clear();
}

void IpAllHashes::add_ip_hemail_to_ip_all_hashes_vec(std::shared_ptr<std::pair<std::string, std::string>> ptr_ip_hemail)
{
    ip_all_hashes_vec_.push_back(ptr_ip_hemail);
}

void IpAllHashes::add_ip_all_hashes_vec_to_ip_all_hashes_2d_mat()
{
    ip_all_hashes_2d_mat_.push_back(ip_all_hashes_vec_);
    ip_all_hashes_vec_.clear();
}

void IpAllHashes::add_ip_all_hashes_2d_mat_to_ip_all_hashes_3d_mat()
{
    ip_all_hashes_3d_mat_.push_back(ip_all_hashes_2d_mat_);
    ip_all_hashes_2d_mat_.clear();
}

std::vector<std::vector<std::vector<std::shared_ptr<std::pair<std::string, std::string>>>>> IpAllHashes::get_ip_all_hashes_3d_mat()
{
    return ip_all_hashes_3d_mat_;
}

void IpAllHashes::remove_front_from_ip_all_hashes_3d_mat()
{
    ip_all_hashes_3d_mat_.front().clear();
}

void IpAllHashes::reset_ip_all_hashes_vec()
{
    ip_all_hashes_vec_.clear();
}

void IpAllHashes::reset_ip_all_hashes_2d_mat()
{
    ip_all_hashes_2d_mat_.clear();
}

void IpAllHashes::reset_ip_all_hashes_3d_mat()
{
    ip_all_hashes_3d_mat_.clear();
}

std::vector<std::shared_ptr<std::pair<std::string, std::string>>> IpHEmail::ip_hemail_vec_ = {};

std::vector<std::shared_ptr<std::pair<std::string, std::string>>> IpAllHashes::ip_all_hashes_vec_ = {};
std::vector<std::vector<std::shared_ptr<std::pair<std::string, std::string>>>> IpAllHashes::ip_all_hashes_2d_mat_ = {};
std::vector<std::vector<std::vector<std::shared_ptr<std::pair<std::string, std::string>>>>> IpAllHashes::ip_all_hashes_3d_mat_ = {};