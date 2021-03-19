#pragma once

#include "json.hpp"

#include <iostream>

namespace Crowd
{
    /**
     * Poco is the consensus algorithm. -- Proof-Of-Chosen-Ones
     */

    class Poco
    {
    public:
        void inform_chosen_ones(std::string my_latest_block, nlohmann::json block_j);
    private:
        //
    };
}