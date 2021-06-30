#include "message_vec.hpp"

using namespace Poco;

void MessageVec::add_to_message_j_vec(nlohmann::json &message_j)
{
    message_j_vec_.push_back(std::make_shared<nlohmann::json>(message_j));
}

std::vector<std::shared_ptr<nlohmann::json>> MessageVec::get_message_j_vec()
{
    return message_j_vec_;
}

void MessageVec::reset_message_j_vec()
{
    message_j_vec_.clear();
}

std::vector<std::shared_ptr<nlohmann::json>> MessageVec::message_j_vec_ = {};