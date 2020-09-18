#pragma once

#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include "configdir.hpp"

using namespace Crowd;

namespace Crowd
{
    /**
     * V is?
     */

    class Verification
    {
    private:
        ConfigDir cd;
        std::string blockchain_folder_path = cd.GetConfigDir() + "blockchain";
    public:
        Verification();
        bool Download();
        bool Update();
    };
}