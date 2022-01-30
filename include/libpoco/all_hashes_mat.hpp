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
        static void add_ip_hemail_to_ip_hemail_vec(std::string participant, std::string hash_of_email);
        static std::vector<std::shared_ptr<std::pair<std::string, std::string>>> get_all_ip_hemail_vec();
        static void reset_ip_hemail_vec();
    private:
        static std::vector<std::shared_ptr<std::pair<std::string, std::string>>> ip_hemail_vec_; // TODO a map is overkill, maybe a pair is better
    };

    class IpAllHashes
    {
    public:
        static void add_ip_hemail_to_ip_all_hashes_vec(std::shared_ptr<std::pair<std::string, std::string>> ip_hemail);
        static void add_ip_all_hashes_vec_to_ip_all_hashes_2d_mat();
        static void add_ip_all_hashes_2d_mat_to_ip_all_hashes_3d_mat();
        static std::vector<std::vector<std::vector<std::shared_ptr<std::pair<std::string, std::string>>>>> get_ip_all_hashes_3d_mat();
        static void remove_front_from_ip_all_hashes_3d_mat();
        static void reset_ip_all_hashes_vec();
        static void reset_ip_all_hashes_2d_mat();
        static void reset_ip_all_hashes_3d_mat();
    private:
        static std::vector<std::shared_ptr<std::pair<std::string, std::string>>> ip_all_hashes_vec_;
        static std::vector<std::vector<std::shared_ptr<std::pair<std::string, std::string>>>> ip_all_hashes_2d_mat_;
        static std::vector<std::vector<std::vector<std::shared_ptr<std::pair<std::string, std::string>>>>> ip_all_hashes_3d_mat_;
    };
}
