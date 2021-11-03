#include "configdir.hpp"

#include "print_or_log.hpp"

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
            Common::Print_or_log pl;
            pl.handle_print_or_log({e.what()});
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

int ConfigDir::CreateDirInConfigDir(std::string directory)
{
    boost::filesystem::path path(configdir + directory);

    if (!boost::filesystem::exists(path))
    {
        try
        {
            boost::filesystem::create_directories(path);
        }
        catch (boost::filesystem::filesystem_error &e)
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({e.what()});
        }
    }

    return 0;
}