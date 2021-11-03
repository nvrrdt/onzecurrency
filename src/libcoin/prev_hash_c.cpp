#include <future>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <vector>
#include <iterator>
#include <algorithm>

#include "prev_hash_c.hpp"
#include "configdir.hpp"
#include "json.hpp"
#include "crypto.hpp"
#include "merkle_tree_c.hpp"

#include "print_or_log.hpp"
#include <boost/lexical_cast.hpp>

using namespace Crowd;
using namespace Coin;

std::string PrevHashC::calculate_hash_from_last_block_c()
{
    std::string prev_hash;

    // read prev_hash file
    ConfigDir cd;
    boost::filesystem::path p (cd.GetConfigDir() + "blockchain/coin");

    Common::Print_or_log pl;

    try
    {
        if (boost::filesystem::exists(p))    // does p actually exist?
        {
            if (boost::filesystem::is_regular_file(p))        // is p a regular file?
            {
                pl.handle_print_or_log({p.string(), "size is", boost::lexical_cast<std::string>(boost::filesystem::file_size(p))});
            }
            else if (boost::filesystem::is_directory(p))      // is p a directory?
            {
                pl.handle_print_or_log({p.string(), "is a directory containing:"});
                
                typedef std::vector<boost::filesystem::path> vec;             // store paths,
                vec v;                                // so we can sort them later

                copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(v));

                sort(v.begin(), v.end());             // sort, since directory iteration
                                                    // is not ordered on some file systems

                // for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
                // {
                //     cout << "   " << *it << '\n';
                // }

                if (v.empty())
                {
                    Coin::merkle_tree_c mt;
                    prev_hash = mt.get_genesis_prev_hash_c();
                }
                else
                {
                    uint64_t n = v.size(); 

                    std::ifstream stream(v[n-1].string(), std::ios::in | std::ios::binary);
                    std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

                    Common::Crypto crypto;
                    prev_hash = crypto.bech32_encode_sha256(contents);
                }
            }
            else
            {
                pl.handle_print_or_log({p.string(), "exists, but is neither a regular file nor a directory"});
            }
        }
        else
        {
            pl.handle_print_or_log({p.string(), "does not exist: create"});

            boost::filesystem::create_directories(p);

            Coin::merkle_tree_c mt;
            prev_hash = mt.get_genesis_prev_hash_c();
        }
    }
    catch (const boost::filesystem::filesystem_error& ex)
    {
        pl.handle_print_or_log({ex.what()});
    }

    return prev_hash;
}

std::vector<std::string> PrevHashC::get_prev_hashes_vec_from_files_c()
{
    std::vector<std::string> prev_hashes;

    ConfigDir cd;
    boost::filesystem::path p (cd.GetConfigDir() + "blockchain/coin");

    Common::Print_or_log pl;

    try
    {
        if (boost::filesystem::exists(p))    // does p actually exist?
        {
            if (boost::filesystem::is_regular_file(p))        // is p a regular file?
            {
                pl.handle_print_or_log({p.string(), "size is", boost::lexical_cast<std::string>(boost::filesystem::file_size(p))});
            }
            else if (boost::filesystem::is_directory(p))      // is p a directory?
            {
                pl.handle_print_or_log({p.string(), "is a directory containing the following:"});

                typedef std::vector<boost::filesystem::path> vec;             // store paths,
                vec v;                                // so we can sort them later

                copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(v));

                sort(v.begin(), v.end());             // sort, since directory iteration
                                                    // is not ordered on some file systems

                for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
                {
                    std::ifstream stream(it->string(), std::ios::in | std::ios::binary);
                    std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
                    nlohmann::json contents_j = nlohmann::json::parse(contents);
                    prev_hashes.push_back(contents_j["prev_hash"]);

                    pl.handle_print_or_log({"prev_hash:", contents_j["prev_hash"]});
                }
            }
            else
            {
                pl.handle_print_or_log({p.string(), "exists, but is neither a regular file nor a directory"});
            }
        }
        else
        {
            pl.handle_print_or_log({p.string(), "does not exist"});
        }
    }
    catch (const boost::filesystem::filesystem_error& ex)
    {
        pl.handle_print_or_log({ex.what()});
    }

    return prev_hashes;
}

std::vector<std::string> PrevHashC::get_blocks_vec_from_files_c()
{
    std::vector<std::string> blocks; // TODO make_shared

    ConfigDir cd;
    boost::filesystem::path p (cd.GetConfigDir() + "blockchain/coin");

    Common::Print_or_log pl;

    try
    {
        if (boost::filesystem::exists(p))    // does p actually exist?
        {
            if (boost::filesystem::is_regular_file(p))        // is p a regular file?
            {
                pl.handle_print_or_log({p.string(), "size is", boost::lexical_cast<std::string>(boost::filesystem::file_size(p))});
            }
            else if (boost::filesystem::is_directory(p))      // is p a directory?
            {
                pl.handle_print_or_log({p.string(), "is a directory containing the following:"});

                typedef std::vector<boost::filesystem::path> vec;             // store paths,
                vec v;                                // so we can sort them later

                copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(v));

                sort(v.begin(), v.end());             // sort, since directory iteration
                                                    // is not ordered on some file systems

                for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
                {
                    std::ifstream stream(it->string(), std::ios::in | std::ios::binary);
                    std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
                    blocks.push_back(contents);

                    //pl.handle_print_or_log({"   blocks:", contents});
                }
            }
            else
            {
                pl.handle_print_or_log({p.string(), "exists, but is neither a regular file nor a directory"});
            }
        }
        else
        {
            pl.handle_print_or_log({p.string(), "does not exist"});
        }
    }
    catch (const boost::filesystem::filesystem_error& ex)
    {
        pl.handle_print_or_log({ex.what()});
    }

    return blocks;
}