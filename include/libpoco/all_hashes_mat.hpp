#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <enet/enet.h>

#include "json.hpp"

namespace Poco
{
    class IpHEmail
    {
    public:
        static void add_to_ip_hemail_vec(enet_uint32 participant, std::string hash_of_email);
        static std::map<enet_uint32, std::shared_ptr<std::vector<std::string>>> get_all_ip_hemail_vec();
        static void reset_ip_hemail_vec();
    private:
        static std::map<enet_uint32, std::shared_ptr<std::vector<std::string>>> ip_hemail_vec_;
    };

    class IpAllHashes
    {
    public:
        static void add_ip_all_hashes_to_ip_all_hashes_vec(nlohmann::json &message_j);
        static void add_ip_all_hashes_vec_to_ip_all_hashes_mat();
        static std::vector<std::vector<std::shared_ptr<nlohmann::json>>> get_ip_all_hashes_mat();
        static void reset_ip_all_hashes_mat();
    private:
        static std::vector<std::shared_ptr<nlohmann::json>> ip_all_hashes_vec_;
        static std::vector<std::vector<std::shared_ptr<nlohmann::json>>> ip_all_hashes_mat_;
    };
}
