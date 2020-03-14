#pragma once

#include <iostream>
#include <string>

namespace crowd
{
    class verification
    {
    public:
        void verification_handler();
        int update_map(std::string);
    private:
        int download_blockchain();
        int update_blockchain();
    };
}