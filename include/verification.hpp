#pragma once

#include <iostream>
#include <string>

namespace crowd
{
    class verification
    {
    public:
        void verification_handler();
        int update_map();
    private:
        int download_blockchain();
        int update_blockchain();
    };
}