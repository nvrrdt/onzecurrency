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
    private:
        int use_log = 1;
    };
}