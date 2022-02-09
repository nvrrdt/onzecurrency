#include "full_hash.hpp"

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include "configdir.hpp"

#include "print_or_log.hpp"

using namespace Crowd;

void FullHash::save_full_hash(std::string& full_hash)
{
    // Saved to file
    ConfigDir cd;
    std::string file = "full_hash";
    cd.CreateFileInConfigDir(file, full_hash);

    // Saved to static variable
    full_hash_ = full_hash;
}

std::string FullHash::get_full_hash()
{
    if (full_hash_ == "")
    {
        // read full_hash file
        ConfigDir cd;
        if (boost::filesystem::exists(cd.GetConfigDir() + "full_hash"))
        {
            std::ifstream stream(cd.GetConfigDir() + "full_hash", std::ios::in | std::ios::binary);
            std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            full_hash_ = contents;
        }
        else
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"Full_hash file doesn't exist!!"});
        }
    }

    return full_hash_;
}