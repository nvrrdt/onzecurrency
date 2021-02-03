#include "configdir.hpp"

using namespace Crowd;

ConfigDir::ConfigDir()
{
    if ((homedir = getenv("HOME")) == NULL)
    {
        homedir = getpwuid(getuid())->pw_dir;
    }
    else
    {
        // char * homedir;
        // getcwd(homedir, sizeof(homedir));
        homedir = "/onzecurrency/build";
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

std::string ConfigDir::GetConfigDir()
{
    return configdir1;
}

int ConfigDir::CreateFileInConfigDir(std::string &filename, std::string &content)
{
    std::ofstream ofs;
    ofs.open (configdir1 + filename, std::ofstream::out | std::ofstream::app);

    ofs << content;

    ofs.close();

    return 0;
}