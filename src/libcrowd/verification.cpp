#include "verification.hpp"

#include "prev_hash.hpp"
#include "crypto.hpp"

using namespace Crowd;

bool Verification::verify_all_blocks()
{
    // Verify the blocks's prev_hash:
    // read first block and create hash
    // read next block and get prev hash --> compare hashes
    // create hash from the last block and continue for the following blocks

    PrevHash ph;
    auto prev_hashes_vec = ph.get_prev_hashes_vec_from_files();
    auto blocks_vec = ph.get_blocks_vec_from_files();

    for (uint64_t i = 0; i < blocks_vec.size(); i++)
    {
        Crypto crypto;

        if (i == blocks_vec.size() - 1) break; // avoid going past prev_hashes_vec

        if (prev_hashes_vec[i+1] == crypto.sha256_create(blocks_vec[i]))
        {
            if (i == blocks_vec.size() - 1 - 1) // because last block can't be verified because of the lack of a prev_hash
            {
                std::cout << "Blockchain verified and ok" << std::endl;
                return true;
            }
            else
            {
                continue;
            }
        }

        std::cout << "Blockchain verified but not ok" << std::endl;
        return false;
    }

    std::cout << "Blockchain not verified and not ok" << std::endl;
    return false;
}
