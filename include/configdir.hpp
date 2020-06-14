#pragma once

#include <iostream>
#include <string>
#include <fstream>

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
            else
            {
                homedir = "";
            }
            
            
            configdir = homedir;
            configdir += "/.config/onzehub/";
        }
        std::string GetConfigDir()
        {
            return configdir;
        }
        int CreateFileInConfigDir(std::string filename, std::string content)
        {
            std::ofstream ofs;
            ofs.open (configdir + filename, std::ofstream::out | std::ofstream::app);

            ofs << content;

            ofs.close();

            return 0;
        }
    };
}