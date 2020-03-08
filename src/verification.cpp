#include <iostream>
#include "verification.hpp"

using namespace crowd;

// step 1: if no blockchain present, download blockchain
// step 2: if blockchain present, but nu map, assemble the map from the blockchain
// step 3: when starting up the software first verify the blockchains hashes
// step 4: after step 3 comes verificiation of the map (users hash as key and some data (unknown yet!) as value)
// step 5: use the map as binary search tree when seeking for a hash above/below (hash_of_chosen_one) withe the hash from the last block (previous_hash of next block)

void verification::verification_handler()
{
    std::cout << "test handler" << std::endl;
}

void verification::download_blockchain()
{
    std::cout << "test download" << std::endl;
};
