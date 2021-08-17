#include "intro_msg_mat.hpp"

using namespace Poco;

void IntroMsgVec::add_to_intro_msg_vec(nlohmann::json &message_j)
{
    intro_msg_vec_.push_back(std::make_shared<nlohmann::json>(message_j));
}

std::vector<std::shared_ptr<nlohmann::json>> IntroMsgVec::get_intro_msg_vec()
{
    return intro_msg_vec_;
}

void IntroMsgVec::reset_intro_msg_vec()
{
    intro_msg_vec_.clear();
}

void IntroMsgsMat::add_intro_msg_to_intro_msg_s_vec(nlohmann::json &message_j)
{
    std::shared_ptr<nlohmann::json> ptr = std::make_shared<nlohmann::json> (message_j);
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

std::vector<std::vector<std::vector<std::shared_ptr<nlohmann::json>>>> IntroMsgsMat::get_intro_msg_s_3d_mat()
{
    return intro_msg_s_3d_mat_;
}

void IntroMsgsMat::replace_intro_msg_s_3d_mat(std::vector<std::vector<std::vector<std::shared_ptr<nlohmann::json>>>> temporary_intro_msg_s_3d_mat)
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

std::vector<std::shared_ptr<nlohmann::json>> IntroMsgVec::intro_msg_vec_ = {};

std::vector<std::shared_ptr<nlohmann::json>> IntroMsgsMat::intro_msg_s_vec_ = {};
std::vector<std::vector<std::shared_ptr<nlohmann::json>>> IntroMsgsMat::intro_msg_s_2d_mat_ = {};
std::vector<std::vector<std::vector<std::shared_ptr<nlohmann::json>>>> IntroMsgsMat::intro_msg_s_3d_mat_ = {};