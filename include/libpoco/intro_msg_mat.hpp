#pragma once

#include <iostream>
#include <vector>

#include "json.hpp"
#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision;

namespace Poco
{
    class IntroMsgMap
    {
    public:
        static void add_to_intro_msg_map(std::string hash, nlohmann::json &message_j);
        static std::map<std::pair<uint256_t, uint256_t>, std::vector<std::shared_ptr<nlohmann::json>>> get_intro_msg_map();
        static void reset_intro_msg_map();
    private:
        static std::map<std::pair<uint256_t, uint256_t>, std::vector<std::shared_ptr<nlohmann::json>>> intro_msg_map_;
    };

    class IntroMsgsMat
    {
    public:
        static void add_intro_msg_to_intro_msg_s_vec(std::pair<std::string, nlohmann::json> &message_j);
        static void add_intro_msg_s_vec_to_intro_msg_s_2d_mat();
        static void add_intro_msg_s_2d_mat_to_intro_msg_s_3d_mat();
        static std::vector<std::vector<std::vector<std::shared_ptr<std::pair<std::string, nlohmann::json>>>>> get_intro_msg_s_3d_mat();
        static void replace_intro_msg_s_3d_mat(std::vector<std::vector<std::vector<std::shared_ptr<std::pair<std::string, nlohmann::json>>>>> temporary_intro_msg_s_3d_mat);
        static void remove_front_from_intro_msg_s_3d_mat();
        static void reset_intro_msg_s_3d_mat();
    private:
        static std::vector<std::shared_ptr<std::pair<std::string, nlohmann::json>>> intro_msg_s_vec_;
        static std::vector<std::vector<std::shared_ptr<std::pair<std::string, nlohmann::json>>>> intro_msg_s_2d_mat_;
        static std::vector<std::vector<std::vector<std::shared_ptr<std::pair<std::string, nlohmann::json>>>>> intro_msg_s_3d_mat_;
    };
}
