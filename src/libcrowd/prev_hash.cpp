#include <future>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <vector>
#include <iterator>
#include <algorithm>


#include "prev_hash.hpp"
#include "configdir.hpp"
#include "json.hpp"
#include "crypto.hpp"

using namespace Crowd;

void PrevHash::save_my_prev_hash_to_file(std::string &prev_hash)
{
    // read prev_hash file
    ConfigDir cd;
    std::string file = "prev_hash";
    cd.CreateFileInConfigDir(file, prev_hash);
}

std::string PrevHash::get_my_prev_hash_from_file()
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

std::string PrevHash::get_last_prev_hash_from_blocks()
{
    std::string prev_hash;

    // read prev_hash file
    ConfigDir cd;
    boost::filesystem::path p (cd.GetConfigDir() + "blockchain");

    try
    {
        if (boost::filesystem::exists(p))    // does p actually exist?
        {
        if (boost::filesystem::is_regular_file(p))        // is p a regular file?
            cout << p << " size is " << boost::filesystem::file_size(p) << '\n';

        else if (boost::filesystem::is_directory(p))      // is p a directory?
        {
            cout << p << " is a directory containing:\n";

            typedef std::vector<boost::filesystem::path> vec;             // store paths,
            vec v;                                // so we can sort them later

            copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(v));

            sort(v.begin(), v.end());             // sort, since directory iteration
                                                // is not ordered on some file systems

            // for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
            // {
            //     cout << "   " << *it << '\n';
            // }

            uint64_t n = v.size(); 

            std::ifstream stream(v[n-1].string(), std::ios::in | std::ios::binary);
            std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

            Crypto crypto;
            prev_hash = crypto.bech32_encode_sha256(contents);
        }
        else
            cout << p << " exists, but is neither a regular file nor a directory\n";
        }
        else
            cout << p << " does not exist\n";
    }

    catch (const boost::filesystem::filesystem_error& ex)
    {
        cout << ex.what() << '\n';
    }




    // if (boost::filesystem::exists(cd.GetConfigDir() + "blockchain"))
    // {
    //     boost::filesystem::path p(cd.GetConfigDir() + "blockchain");

    //     typedef std::vector<boost::filesystem::path> vec;
    //     vec v;

    //     std::copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(v));

    //     std::sort(v.begin(), v.end()
    //         , [](boost::filesystem::path const& a, boost::filesystem::path const& b) {
    //         return std::stoi(a.filename().string()) < std::stoi(b.filename().string());
    //     });

    //     uint64_t n = v.size(); 

    //     std::ifstream stream(v[n-1].string(), std::ios::in | std::ios::binary);
    //     std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

    //     nlohmann::json json = nlohmann::json::parse(contents);
    //     std::string json_s = json.dump();

    //     Crypto crypto;
    //     prev_hash = crypto.bech32_encode_sha256(json_s);
    // }
    // else
    // {
    //     std::cout << "Prev_hash not found in blockchain!!" << std::endl;
    // }

    return prev_hash;
}