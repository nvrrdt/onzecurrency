#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace Crowd
{
    /**
     * Verificiation of the integrity of the blockchain 
     */

    class Verification
    {
    public:
        bool verify_all_blocks();
        bool compare_email_with_saved_full_hash(std::string &email_address);
    private:
    };
}