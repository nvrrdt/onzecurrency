#pragma once

#include <iostream>
#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

namespace Crowd
{
    /**
     * Gives the ~/.config/onzehub directory
     */

    class ConfigDir
    {
        char *homedir;
        std::string configdir;
    public:
        ConfigDir()
        {
            if ((homedir = getenv("HOME")) == NULL)
            {
                homedir = getpwuid(getuid())->pw_dir;
            }
            
            configdir = homedir;
        }
        virtual std::string GetConfigDir()
        {
            configdir += "/.config/onzehub/";

            return configdir;
        }
    };
}