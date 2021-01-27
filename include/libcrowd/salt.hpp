#pragma once

#include <iostream>
#include <string>

#include "random.hpp"

namespace Crowd
{
    class Salt
    {
    public:
        std::string create_and_save_salt_to_file();
        std::string get_salt_from_file();
    private:
        std::string salt_;
    };
}