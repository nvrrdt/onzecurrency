#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <enet/enet.h>

namespace Poco
{
    class AllHashes
    {
    public:
        static void add_to_all_hashes(enet_uint32 participant, std::string full_hash_req, std::string prev_hash_req);
        static std::map<enet_uint32, std::shared_ptr<std::vector<std::string>>> get_all_hashes();
        static void reset_all_hashes();
    private:
        static std::map<enet_uint32, std::shared_ptr<std::vector<std::string>>> all_hashes_;
    };
}
