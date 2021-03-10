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
        homedir = "/onzecurrency/";
    }
    
    
    configdir = homedir;
    configdir = configdir + ".config/onzehub/";

    boost::system::error_code c;
    boost::filesystem::path path(configdir);

    if (!boost::filesystem::exists(path))
    {
        try
        {
            boost::filesystem::create_directories(path);
        }
        catch (boost::filesystem::filesystem_error &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}

std::string ConfigDir::GetConfigDir()
{
    return configdir;
}

int ConfigDir::CreateFileInConfigDir(std::string &filename, std::string &content)
{
    std::ofstream ofs;
    ofs.open (configdir + filename, std::ofstream::out | std::ofstream::app);

    ofs << content;

    ofs.close();

    return 0;
}