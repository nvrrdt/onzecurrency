#pragma once

#include <iostream>
#include <string>

namespace crowd
{
    class verification
    {
    public:
        void verification_handler(bool);
        void update_map();
    private:
        void download_blockchain(bool);
        void update_blockchain();
    };
}