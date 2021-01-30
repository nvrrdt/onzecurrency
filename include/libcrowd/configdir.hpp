#pragma once

#include <iostream>
#include <string>
#include <fstream>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

namespace Crowd
{
    /**
     * Gives the ~/.config/onzehub directory
     */

    class ConfigDir
    {
    public:
        ConfigDir();
        std::string GetConfigDir();
        int CreateFileInConfigDir(std::string &filename, std::string &content);
    private:
        const char *homedir;
        std::string configdir;
        std::string configdir1;
    };
}