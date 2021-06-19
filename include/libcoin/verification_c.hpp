#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "verification.hpp"

using namespace Crowd;

namespace Coin
{
    /**
     * Verificiation of the integrity of the blockchain 
     */

    class VerificationC: Verification
    {
    public:
        bool verify_all_blocks();
    private:
    };
}