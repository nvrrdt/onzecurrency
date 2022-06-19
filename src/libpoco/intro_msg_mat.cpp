#include "intro_msg_mat.hpp"

#include "sharding.hpp"

using namespace Poco;

void IntroMsgMap::add_to_intro_msg_map(std::string hash, nlohmann::json &message_j)
{
    // I want the messages here distributed into the right shard
    // so it can easily be processed in poco_crowd's create_prel_blocks()
    // so intro_msg_vec must become intro_msg_map
    Poco::DatabaseSharding ds;
    intro_msg_map_[ds.get_fair_shard_range(hash)] = std::make_shared<nlohmann::json>(message_j);
}

std::map<std::pair<uint256_t, uint256_t>, std::shared_ptr<nlohmann::json>> IntroMsgMap::get_intro_msg_map()
{
    return intro_msg_map_;
}

void IntroMsgMap::reset_intro_msg_map()
{
    intro_msg_map_.clear();
}

void IntroMsgsMat::add_intro_msg_to_intro_msg_s_vec(std::pair<std::string, nlohmann::json> &message_j)
{
    std::shared_ptr<std::pair<std::string, nlohmann::json>> ptr = std::make_shared<std::pair<std::string, nlohmann::json>> (message_j);
    intro_msg_s_vec_.push_back(ptr);
}

void IntroMsgsMat::add_intro_msg_s_vec_to_intro_msg_s_2d_mat()
{
    intro_msg_s_2d_mat_.push_back(intro_msg_s_vec_);
    intro_msg_s_vec_.clear();
}

void IntroMsgsMat::add_intro_msg_s_2d_mat_to_intro_msg_s_3d_mat()
{
    intro_msg_s_3d_mat_.push_back(intro_msg_s_2d_mat_);
    intro_msg_s_2d_mat_.clear();
}

std::vector<std::vector<std::vector<std::shared_ptr<std::pair<std::string, nlohmann::json>>>>> IntroMsgsMat::get_intro_msg_s_3d_mat()
{
    return intro_msg_s_3d_mat_;
}

void IntroMsgsMat::replace_intro_msg_s_3d_mat(std::vector<std::vector<std::vector<std::shared_ptr<std::pair<std::string, nlohmann::json>>>>> temporary_intro_msg_s_3d_mat)
{
    intro_msg_s_3d_mat_.clear();
    intro_msg_s_3d_mat_ = temporary_intro_msg_s_3d_mat;
}

void IntroMsgsMat::remove_front_from_intro_msg_s_3d_mat()
{
    intro_msg_s_3d_mat_.front().clear();
}

void IntroMsgsMat::reset_intro_msg_s_3d_mat()
{
    intro_msg_s_3d_mat_.clear();
}

std::map<std::pair<uint256_t, uint256_t>, std::shared_ptr<nlohmann::json>> IntroMsgMap::intro_msg_map_ = {};

std::vector<std::shared_ptr<std::pair<std::string, nlohmann::json>>> IntroMsgsMat::intro_msg_s_vec_ = {};
std::vector<std::vector<std::shared_ptr<std::pair<std::string, nlohmann::json>>>> IntroMsgsMat::intro_msg_s_2d_mat_ = {};
std::vector<std::vector<std::vector<std::shared_ptr<std::pair<std::string, nlohmann::json>>>>> IntroMsgsMat::intro_msg_s_3d_mat_ = {};