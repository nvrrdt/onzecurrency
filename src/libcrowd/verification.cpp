#include "verification.hpp"

#include "prev_hash.hpp"
#include "full_hash.hpp"
#include "crypto.hpp"

#include "print_or_log.hpp"

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

    Common::Print_or_log pl;

    for (uint64_t i = 0; i < blocks_vec.size(); i++)
    {
        Common::Crypto crypto;

        if (i == blocks_vec.size() - 1) break; // avoid going past prev_hashes_vec

        bool out_success; // not used here!
        if (crypto.bech32_decode(prev_hashes_vec[i+1], out_success) == crypto.sha256_create(blocks_vec[i]))
        {
            if (i == blocks_vec.size() - 1 - 1) // because last block can't be verified because of the lack of a prev_hash
            {
                pl.handle_print_or_log({"Blockchain verified and ok"});
                return true;
            }
            else
            {
                continue;
            }
        }

        pl.handle_print_or_log({"Blockchain verified but not ok"});
        return false;
    }

    pl.handle_print_or_log({"Blockchain not verified and not ok, it may be there's only 1 block"});
    return false;
}

bool Verification::compare_email_with_saved_full_hash(std::string & email_address)
{
    Common::Print_or_log pl;
    Common::Crypto crypto;
    std::string hash_email = crypto.bech32_encode_sha256(email_address);
    PrevHash ph;
    std::string prev_hash = ph.get_my_prev_hash_from_file();
    std::string email_prev_hash_app = hash_email + prev_hash; // TODO should this anonymization not be numbers instead of strings?
    std::string full_hash_calc = crypto.bech32_encode_sha256(email_prev_hash_app);

    FullHash fh;
    std::string full_hash_from_file =  fh.get_full_hash_from_file();
// pl.handle_print_or_log({"prev calc: " << prev_hash});
// pl.handle_print_or_log({"full calc:", full_hash_calc, ", full file:", full_hash_from_file});
    if (full_hash_calc == full_hash_from_file)
    {
        pl.handle_print_or_log({"Full_hashes compared and ok"});
        return true;
    }
    else
    {
        pl.handle_print_or_log({"Full_hashes compared and not ok"});
        return false;
    }
}