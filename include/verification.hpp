#pragma once

#include <iostream>
#include <string>

namespace crowd
{
    class verification
    {
    public:
        void verification_handler();
        void update_map();
    private:
        void download_blockchain();
        void update_blockchain();
        void authentication();
    };
}