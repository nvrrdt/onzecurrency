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
        const char *homedir;
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
                /* char cwd[1024];
                getcwd(cwd, sizeof(cwd));
                std::string hd = cwd;
                homedir = hd.c_str(); */
                homedir = "."; // use git clone onzehub-dev-setup with docker
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