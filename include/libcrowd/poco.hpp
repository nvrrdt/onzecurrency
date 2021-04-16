#pragma once

#include "json.hpp"

#include <iostream>
#include <enet/enet.h>

namespace Crowd
{
    /**
     * Poco is the consensus algorithm. -- Proof-Of-Chosen-Ones
     */

    class Poco
    {
    public:
        void inform_chosen_ones(std::string my_next_block, nlohmann::json block_j, std::map<enet_uint32, std::string> &all_full_hashes);
    private:
        //
    };
}