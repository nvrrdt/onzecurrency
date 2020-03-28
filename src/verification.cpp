#include <iostream>
#include <string>
#include "verification.hpp"

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include <iomanip>
#include "json.hpp"

#include <boost/range/iterator_range.hpp>
#include <fstream>

#include "merkle_tree.hpp"
#include "p2p_handler.hpp"
#include "ping.hpp"

using namespace crowd;
using namespace boost::filesystem;

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
        std::cout << "Error Response: " << c << std::endl; // TODO: create directory!
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
            verification::update_map();
        }
    }
}

void verification::download_blockchain()
{
    std::cout << "test download" << std::endl;

    // TODO: get ip adresses from ip_peers.json, ping a random peer for being online, download for 2 minutes from that random peer, then another random peer
    p2p_handler ph;
    ph.p2p_switch("download"); // TODO: get some data from the server
};

void verification::update_blockchain()
{
    std::cout << "test blockchain update" << std::endl;
}

void verification::update_map()
{
    std::cout << "test map update" << std::endl;

    // create map by reading the blocks in the blockchain map
    boost::system::error_code c;
    boost::filesystem::path path("../blockchain");
    bool isDir = boost::filesystem::is_directory(path, c);

    if(!isDir)
    {
        std::cout << "Error Response: " << c << std::endl;
    }
    else
    {
        // make for every directory entry a hashed map entry made by concatenating and hashing email_h + passw_h
        std::map<std::string, std::string> map_of_users;

        for (auto& entry : boost::make_iterator_range(directory_iterator(path), {}))
        {
            std::cout << entry << "\n";
            std::ifstream ifile (entry.path().string(), std::ios::in);

            nlohmann::json j;
            ifile >> j;

            std::cout << std::setw(4) << j << std::endl;

            // Concatenate email_h and passw_h and hash that string, the resulting hash is the key for the map
            // What should be the value? online presence (standard not online TODO restart app after creating_user && user_in_block), ...

            for (auto& element : j["data"]) {
                std::cout << element["email_h"] << '\n';
                std::cout << element["passw_h"] << '\n';

                // TODO TEST: verify if user_conc_hash here is the same as user_hashed in block.cpp
                std::string user_conc = string(element["email_h"][0]) + string(element["passw_h"][0]), user_conc_hash;
                merkle_tree mt;
                if (mt.create_hash(user_conc, user_conc_hash) == true)
                {
                    std::cout << user_conc << std::endl;
                    std::cout << user_conc_hash << std::endl;

                    map_of_users.insert(std::make_pair(user_conc_hash, "not_online"));
                }
            }
        }

        // TODO: I thought of saving the map to a file, but I didn't succeed in finding an example
        // and I either don't see an advantage, just load it in ram at program start
    }
}