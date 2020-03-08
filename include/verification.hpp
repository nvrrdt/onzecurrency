#pragma once

namespace crowd
{
    class verification
    {
    public:
        void verification_handler();
    private:
        int download_blockchain();
        int update_blockchain();
        int update_map();
    };
}