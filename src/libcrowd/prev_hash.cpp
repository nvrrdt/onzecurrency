#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include "prev_hash.hpp"
#include "configdir.hpp"
#include "random.hpp"

using namespace Crowd;

void PrevHash::save_prev_hash_to_file(std::string &prev_hash)
{
    // read prev_hash file
    ConfigDir cd;
    std::string file = "prev_hash";
    cd.CreateFileInConfigDir(file, prev_hash);
}

std::string PrevHash::get_prev_hash_from_file()
{
    std::string ph;

    // read prev_hash file
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "prev_hash"))
    {
        // get prev_hash
        std::ifstream stream(cd.GetConfigDir() + "prev_hash", std::ios::in | std::ios::binary);
        std::string prev_hash((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        ph = prev_hash;
    }
    else
    {
        std::cout << "Prev_hash file doesn't exist!!" << std::endl;
    }

    return ph;
}