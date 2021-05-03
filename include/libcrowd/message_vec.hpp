#pragma once

#include <iostream>
#include <vector>

#include "json.hpp"

namespace Crowd
{
    class MessageVec
    {
    public:
        MessageVec();
        static void add_to_message_j_vec(nlohmann::json &message_j);
        static std::vector<std::shared_ptr<nlohmann::json>> get_message_j_vec();
        static void reset_message_j_vec();
    private:
        static std::vector<std::shared_ptr<nlohmann::json>> message_j_vec_;
    };
}
