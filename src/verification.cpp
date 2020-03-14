#include <iostream>
#include "verification.hpp"

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

using namespace crowd;

// step 1: if no blockchain present, download blockchain
// step 2: if blockchain present, but nu map, assemble the map from the blockchain
// step 3: when starting up the software first verify the blockchains hashes
// step 4: after step 3 comes verificiation of the map (users hash as key and some data (unknown yet!) as value)
// step 5: use the map as binary search tree when seeking for a hash above/below (hash_of_chosen_one) withe the hash from the last block (previous_hash of next block)

void verification::verification_handler()
{
    // verify if blockchain folder is empty and if it is download the blockchain, if it's not empty verify the files to verify the blockchain and the map
    boost::system::error_code c;
    boost::filesystem::path path("../blockchain");
    bool isDir = boost::filesystem::is_directory(path, c);
    bool isEmpty = boost::filesystem::is_empty(path);

    if(!isDir)
    {
        std::cout << "Error Response: " << c << std::endl;
    }
    else
    {
        if (!isEmpty)
        {
            std::cout << "Directory not empty" << std::endl;
            verification::update_blockchain();
            verification::update_map();
        }
        else
        {
            std::cout << "Is a directory, is empty" << std::endl;
            verification::download_blockchain();
        }
    }
}

int verification::download_blockchain()
{
    std::cout << "test download" << std::endl;

    return 0;
};

int verification::update_blockchain()
{
    std::cout << "test blockchain update" << std::endl;

    return 0;
}

int verification::update_map()
{
    std::cout << "test map update" << std::endl;

    return 0;
}