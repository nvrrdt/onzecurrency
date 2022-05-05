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
    private:
        int use_log = 1;
        int amount_of_chosen_ones = 128;
    };
}