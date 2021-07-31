#pragma once

#include <iostream>
#include <vector>

#include "json.hpp"

namespace Poco
{
    class IntroMsgVec
    {
    public:
        static void add_to_intro_msg_vec(nlohmann::json &message_j);
        static std::vector<std::shared_ptr<nlohmann::json>> get_intro_msg_vec();
        static void reset_intro_msg_vec();
    private:
        static std::vector<std::shared_ptr<nlohmann::json>> intro_msg_vec_;
    };

    class IntroMsgsMat
    {
    public:
        static void add_intro_msg_to_intro_msg_s_vec(nlohmann::json &message_j);
        static void add_intro_msg_s_vec_to_intro_msg_s_mat();
        static std::vector<std::vector<std::shared_ptr<nlohmann::json>>> get_intro_msg_s_mat();
        static void reset_intro_msg_s_mat();
    private:
        static std::vector<std::shared_ptr<nlohmann::json>> intro_msg_s_vec_;
        static std::vector<std::vector<std::shared_ptr<nlohmann::json>>> intro_msg_s_mat_;
    };
}
