#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include "salt.hpp"
#include "configdir.hpp"
#include "random.hpp"

using namespace Crowd;

std::string Salt::create_and_save_salt_to_file()
{
    // create salt
    Random r;
    salt_ = r.get_random_number();

    // read salt file
    ConfigDir cd;
    std::string file = "salt";
    cd.CreateFileInConfigDir(file, salt_);

    return salt_;
}

std::string Salt::get_salt_from_file()
{
    std::string salt;

    // read salt file
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "salt"))
    {
        // get salt
        std::ifstream stream(cd.GetConfigDir() + "salt", std::ios::in | std::ios::binary);
        std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        for(auto i: contents) {
            int value = i;
            //std::cout << "data: " << value << std::endl;
        }

        salt = contents;
    }
    else
    {
        std::cout << "Salt file doesn't exist!!" << std::endl;
    }

    return salt;
}