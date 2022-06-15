#pragma once

namespace Common
{
    class Globals
    {
    public:
        int get_use_log()
        {
            return use_log;
        }
        int get_amount_of_chosen_ones()
        {
            return amount_of_chosen_ones;
        }
        int get_max_pow()
        {
            return max_pow;
        }
        int get_block_time()
        {
            return block_time;
        }
    private:
        int use_log = 1;
        int amount_of_chosen_ones = 128;
        int max_pow = 7; // 2^max_pow = amount_of_shards
        int block_time = 64; // must be 2^x
    };
}