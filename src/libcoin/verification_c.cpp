#include "verification_c.hpp"

#include "prev_hash_c.hpp"

#include "crypto.hpp"

#include "print_or_log.hpp"

using namespace Coin;

bool VerificationC::verify_all_blocks()
{
    // Verify the blocks's prev_hash:
    // read first block and create hash
    // read next block and get prev hash --> compare hashes
    // create hash from the last block and continue for the following blocks

    Common::Print_or_log pl;

    PrevHashC ph;
    auto prev_hashes_vec = ph.get_prev_hashes_vec_from_files_c();
    auto blocks_vec = ph.get_blocks_vec_from_files_c();

    if (blocks_vec.empty())
    {
        pl.handle_print_or_log({"Blockchain coin verified and ok"});
        return true;
    }

    for (uint64_t i = 0; i < blocks_vec.size(); i++)
    {
        Common::Crypto crypto;

        if (i == blocks_vec.size() - 1) break; // avoid going past prev_hashes_vec

        bool out_success; // not used here!
        if (crypto.bech32_decode(prev_hashes_vec[i+1], out_success) == crypto.sha256_create(blocks_vec[i]))
        {
            if (i == blocks_vec.size() - 1 - 1) // because last block can't be verified because of the lack of a prev_hash
            {
                pl.handle_print_or_log({"Blockchain coin verified and ok"});
                
                return true;
            }
            else
            {
                continue;
            }
        }

        pl.handle_print_or_log({"Blockchain coin verified but not ok"});
        
        return false;
    }

    pl.handle_print_or_log({"Blockchain coin not verified and not ok, it may be there's only 1 block"});
        
    return false;
}
