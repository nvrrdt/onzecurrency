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
        const char *homedir;
        std::string configdir;
        std::string configdir1;
    public:
        ConfigDir()
        {
            if ((homedir = getenv("HOME")) == NULL)
            {
                homedir = getpwuid(getuid())->pw_dir;
            }
            else
            {
                char cwd[1024];
                getcwd(cwd, sizeof(cwd));
                std::string hd = cwd;
                homedir = hd.c_str();
                homedir = "../build"; // use git clone onzehub-dev-setup with docker
            }
            
            
            configdir = homedir;
            configdir += "/.config/";
            configdir1 = configdir + "onzehub/";

            boost::system::error_code c;
            boost::filesystem::path path(configdir);
            boost::filesystem::path path1(configdir1);

            if (!boost::filesystem::exists(path1))
            {
                try
                {
                    boost::filesystem::create_directory(path);
                    boost::filesystem::create_directory(path1);
                }
                catch (boost::filesystem::filesystem_error &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }
        std::string GetConfigDir()
        {
            return configdir1;
        }
        int CreateFileInConfigDir(std::string filename, std::string content)
        {
            std::ofstream ofs;
            ofs.open (configdir1 + filename, std::ofstream::out | std::ofstream::app);

            ofs << content;

            ofs.close();

            return 0;
        }
    };
}