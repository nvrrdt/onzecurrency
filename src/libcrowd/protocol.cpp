#include <future>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <vector>
#include <sstream>

#include "json.hpp"

#include "p2p.hpp"
#include "rocksy.hpp"
#include "configdir.hpp"
#include "crypto.hpp"

#include "print_or_log.hpp"
#include <boost/lexical_cast.hpp>

using namespace Crowd;

std::string Protocol::get_last_block_nr()
{
    ConfigDir cd;
    std::string blockchain_folder_path = cd.GetConfigDir() + "blockchain/crowd";
    boost::system::error_code c;
    boost::filesystem::path path(blockchain_folder_path);

    std::ostringstream o;

    if (!boost::filesystem::exists(path))
    {
        return "no blockchain present in folder";
    }
    else
    {
        typedef std::vector<boost::filesystem::path> vec;             // store paths,
        vec v;                                // so we can sort them later

        copy(boost::filesystem::directory_iterator(path), boost::filesystem::directory_iterator(), back_inserter(v));

        sort(v.begin(), v.end());             // sort, since directory iteration
                                            // is not ordered on some file systems

        // for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
        // {
        //     cout << "   " << *it << '\n';
        // }

        uint64_t n = v.size() - 1; // TODO: perhaps verify with the number in de filename

        o << n;
    }
    
    return o.str(); // TODO: also verify latest hash
}

std::map<uint32_t, uint256_t> Protocol::layers_management(uint256_t &amount_of_peers)
{
    Common::Print_or_log pl;

    Common::Globals globals;
    const int nmax = globals.get_amount_of_chosen_ones();   // max number of peers per section
    uint32_t maxreallayers;             // number of layers
    uint256_t tmaxmax = 0, tmaxmin = 0, overshoot = 0, tlayer = 0;

    std::vector<uint256_t> layercontents;

    int imax = 64;
    for (maxreallayers = 1; maxreallayers <= imax; maxreallayers++)
    {
        
        tlayer = uint256_t(pow(nmax, maxreallayers));   // max number of peers in a layer
        tmaxmax += tlayer;                  // max number of peers in all layers

        layercontents.push_back(tlayer);

        if (amount_of_peers <= tmaxmax)
        {
            // pl.handle_print_or_log({"max peers last layer:", std::to_string(tlayer)});
            // pl.handle_print_or_log({"max peers all layers:", std::to_string(tmaxmax)});
            pl.handle_print_or_log({"amount of layers:", std::to_string(maxreallayers)});
            pl.handle_print_or_log({"amount_of_peers:", amount_of_peers.str()});
            break;
        }
        else if (maxreallayers == imax && amount_of_peers > tmaxmax)
        {
            pl.handle_print_or_log({"ERROR: change imax as there are more peers than imax can handle"});
        }
    }

    tmaxmin = tmaxmax - layercontents.at(maxreallayers-1);  // max number of peers in all layers minus last layer
    overshoot = amount_of_peers - tmaxmin;                  // number of peers in last layer
    
    // partitioning of last layer:
    // explanation: (remainder) contains (quotient + 1) peers in 1 bucket of last layer
    //              (layercontents(last_layer) - remainder) contains (quotient) peers in 1 bucket of last layer
    //              (layercontents(last_layer)) is maximum number of peers in that layer
    uint256_t quotient, remainder;
    if (maxreallayers == 1) // only 1 layer == layer 0
    {
        quotient = 0;
        remainder = overshoot;
        // pl.handle_print_or_log({"quotient:", std::to_string(quotient)});
        // pl.handle_print_or_log({"remainder:", std::to_string(remainder)});
    }
    else
    {
        quotient = overshoot / layercontents.at(maxreallayers-1);
        remainder = overshoot % layercontents.at(maxreallayers-1);
        // pl.handle_print_or_log({std::to_string(remainder), "buckets in last layer with", std::to_string(quotient+1), "peers and", std::to_string((layercontents.at(maxreallayers-2))-remainder), "buckets with", std::to_string(quotient), "peers."});
        // pl.handle_print_or_log({std::to_string(overshoot), "(overshoot) must be equal to", std::to_string((remainder*(quotient+1))+((layercontents.at(maxreallayers-2)-remainder)*quotient)), "(calculation of here before mentioned numbers)"});
    }

    // pl.handle_print_or_log({"numberlayers:" << std::to_string(maxreallayers)});
    // pl.handle_print_or_log({"numbermaxbucketslastlayer:", std::to_string(remainder)});
    // pl.handle_print_or_log({"contentsmaxbucketslastlayer:", std::to_string(quotient+1)});
    // pl.handle_print_or_log({"numberminbucketslastlayer:", ((maxreallayers == 1) ? std::to_string(100-remainder) : std::to_string((layercontents.at(maxreallayers-1)-remainder)))});
    // pl.handle_print_or_log({"contentsminbucketslastlayer:", std::to_string(quotient)});
    // pl.handle_print_or_log({"overshootlastlayer:", std::to_string(overshoot)});

    // the peers between every top layer peer are counted and put in a map
    uint256_t counter, total_counter;
    std::map<uint32_t, uint256_t> chosen_ones_counter;

    for (int i = 1; i <= nmax; i++) // nmax = amount of chosen ones
    {
        chosen_ones_counter[i] = 0;
    }
    for (int i = 1; i <= nmax; i++)
    {
        for (int layer = 1; layer <= maxreallayers; layer++)
        {
            if (maxreallayers == 1)
            {
                for (int m = 1; m <= overshoot; m++)
                {
                    chosen_ones_counter[m] = 1;
                }
            }
            else if (maxreallayers > 1)
            {
                if (layer == 1)
                {
                     chosen_ones_counter[i] += 1;
                }
                else if (layer < maxreallayers)
                {
                    chosen_ones_counter[i] += layercontents.at(layer-1) / nmax;
                }
                else if (layer == maxreallayers)
                {
                    if (remainder <= (layercontents.at(maxreallayers-1)) / nmax && remainder != 0)
                    {
                        chosen_ones_counter[i] += quotient+1 * remainder;
                        remainder = 0;
                        break;
                    }
                    else if (remainder > (layercontents.at(maxreallayers-1)) / nmax)
                    {
                        chosen_ones_counter[i] += quotient+1 * (layercontents.at(maxreallayers-1) / nmax);
                        remainder -= (layercontents.at(maxreallayers-1) / nmax);
                        break;
                    }
                    else
                    {
                        chosen_ones_counter[i] += quotient * (layercontents.at(maxreallayers-1) / nmax);
                        break;
                    }
                }
            }
        }
        // pl.handle_print_or_log({"coc:", std::to_string(i), std::to_string(chosen_ones_counter[i])});
        // total_counter += chosen_ones_counter[i];
    }
    pl.handle_print_or_log({"amount of peers to verify:", total_counter.str()});

    return chosen_ones_counter;
}

std::map<int, std::string> Protocol::partition_in_buckets(std::string &my_hash, std::string &next_hash)
{
    // the input is rocksdb, where you will get the total amount of peers from
    // starting with the hash of the chosen one
    // less than 100 peers in rocksdb and every one is the chosen one or coordinator
    // more than 1 layer and you have to count and partition in buckets with their respective coordinator

    // count peers between my_hash and next_hash in rocksdb, then calculate the layers with an adapted layers_management
    // t.client({"msg": "communicate_to", "you_hash": "you_hash", "next_hash": "next_hash"}) --> at receiving server: calculate your bucket
    // 1 layer: t.client({"msg": "communicate_to", "you_hash": "1", "next_hash": "2"})
    // 2 layers, layer 1: t.client({"msg": "communicate_to", "you_hash": "1", "next_hash": "102"}) --> calculate preliminary peers between hashes
                                                                                                    // based on the current state of rocksdb
    // 2 layers, layer 2: t.client({"msg": "communicate_to", "you_hash": "103", "next_hash": "104"})

    Rocksy *rocksy = new Rocksy("usersdbreadonly");
    uint256_t count = rocksy->CountPeersFromTo(my_hash, next_hash);
    delete rocksy;

    std::map<uint32_t, uint256_t> chosen_ones_counter = Protocol::layers_management(count);
    auto h = Protocol::get_calculated_hashes(my_hash, chosen_ones_counter);

    return h;
}

std::map<int, std::string> Protocol::get_calculated_hashes(std::string &my_hash, std::map<uint32_t, uint256_t> &chosen_ones_counter)
{
    // Get the hashes from rocksdb, the count is accessible now
    
    std::map<int, std::string> calculated_hashes;
    std::string the_hash;
    Rocksy* rocksy = new Rocksy("usersdbreadonly");
    for (int i = 1; i <= chosen_ones_counter.size(); i++)
    {
        uint256_t val = chosen_ones_counter[i];

        if (val == 0) continue;

        // the last counted hash should be approximately my_hash in partition_in_buckets(my_hash, my_hash)
        // when chosen_ones_counter.size() is 2 you don't need to continue

        if (i == 1)
        {
            calculated_hashes[i] = my_hash;
            the_hash = rocksy->FindPeerFromTillCount(my_hash, val);

            if (chosen_ones_counter.size() == 2) break;
        }
        else
        {
            calculated_hashes[i] = the_hash;
            the_hash = rocksy->FindPeerFromTillCount(the_hash, val);
        }
    }
    delete rocksy;
    return calculated_hashes;
}

nlohmann::json Protocol::get_blocks_from(std::string &latest_block_peer)
{
    nlohmann::json all_blocks_j;

    ConfigDir cd;
    boost::filesystem::path p (cd.GetConfigDir() + "blockchain/crowd");

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
                pl.handle_print_or_log({p.string(), "is a directory containing the next:"});

                typedef std::vector<boost::filesystem::path> vec;             // store paths,
                vec v;                                // so we can sort them later

                copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(v));

                sort(v.begin(), v.end());             // sort, since directory iteration
                                                    // is not ordered on some file systems

                nlohmann::json block_j;
                std::uint64_t value_peer;
                std::istringstream isss(latest_block_peer);
                isss >> value_peer;
                for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
                {
                    pl.handle_print_or_log({"   ", (*it).string()});

                    std::string str = it->stem().string();
                    std::uint64_t value_this_blockchain_dir;
                    std::istringstream iss(str.substr(6,18));
                    iss >> value_this_blockchain_dir;
                    if (latest_block_peer != "no blockchain present in folder")
                    {
                        pl.handle_print_or_log({"value_this_blockchain_dir:", std::to_string(value_this_blockchain_dir), "value_peer:", std::to_string(value_peer)});
                        if (value_this_blockchain_dir == value_peer)
                        {
                            std::ifstream stream(it->string(), std::ios::in | std::ios::binary);
                            std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
                            nlohmann::json contents_j = nlohmann::json::parse(contents);
                            block_j["block"] = contents_j;
                            std::ostringstream oss;
                            oss << value_peer;
                            block_j["block_nr"] = oss.str();
                            all_blocks_j[value_peer] = block_j;

                            value_peer++;
                        }
                    }
                    else
                    {
                        std::ifstream stream(it->string(), std::ios::in | std::ios::binary);
                        std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
                        nlohmann::json contents_j = nlohmann::json::parse(contents);
                        block_j["block"] = contents_j;
                        std::ostringstream oss;
                        oss << value_this_blockchain_dir;
                        block_j["block_nr"] = oss.str();
                        all_blocks_j[value_this_blockchain_dir] = block_j;
                    }
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

    return all_blocks_j;
}

std::string Protocol::get_all_users_from(std::string &latest_block_peer)
{
    nlohmann::json all_users;

    ConfigDir cd;
    boost::filesystem::path p (cd.GetConfigDir() + "blockchain/crowd");

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
                pl.handle_print_or_log({p.string(), "is a directory containing the next:"});

                typedef std::vector<boost::filesystem::path> vec;             // store paths,
                vec v;                                // so we can sort them later

                copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(v));

                sort(v.begin(), v.end());             // sort, since directory iteration
                                                    // is not ordered on some file systems

                nlohmann::json block_j;
                for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
                {
                    pl.handle_print_or_log({"   ", (*it).string()});

                    std::string str = it->stem().string();
                    std::uint64_t value_this_blockchain_dir;
                    std::istringstream iss(str.substr(6,18));
                    iss >> value_this_blockchain_dir;
                    std::uint64_t value_peer;

                    std::istringstream isss(latest_block_peer);
                    isss >> value_peer;
                    pl.handle_print_or_log({"value_this_blockchain_dir:", std::to_string(value_this_blockchain_dir), "value_peer:", std::to_string(value_peer)});
                    if (value_this_blockchain_dir >= value_peer)
                    {
                        std::ifstream stream(it->string(), std::ios::in | std::ios::binary);
                        std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
                        nlohmann::json contents_j = nlohmann::json::parse(contents);

                        for (auto& element : contents_j["entry"])
                        {
                            std::string full_hash;
                            full_hash = element["full_hash"];

                            all_users.push_back(full_hash);
                        }
                    }
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

    return all_users.dump();
}

std::string Protocol::block_plus_one(std::string &block_nr)
{
    uint64_t value;
    std::istringstream iss(block_nr);
    iss >> value;
    std::ostringstream o;
    o << value + 1;
    std::string new_block = o.str();

    return new_block;
}

nlohmann::json Protocol::get_block_at(std::string block_nr_s)
{
    ConfigDir cd;
    std::string blockchain_folder_path = cd.GetConfigDir() + "blockchain/crowd";
    boost::system::error_code c;
    boost::filesystem::path path(blockchain_folder_path);

    nlohmann::json block_j;

    std::uint64_t block_nr;
    std::istringstream i(block_nr_s);
    i >> block_nr;

    if (!boost::filesystem::exists(path))
    {
        return "no blockchain present in folder";
    }
    else
    {
        typedef std::vector<boost::filesystem::path> vec;             // store paths,
        vec v;                                // so we can sort them later

        copy(boost::filesystem::directory_iterator(path), boost::filesystem::directory_iterator(), back_inserter(v));

        sort(v.begin(), v.end());             // sort, since directory iteration
                                            // is not ordered on some file systems

        for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
        {
            if (it == v.begin() + block_nr)
            {
                std::ifstream stream(it->string(), std::ios::in | std::ios::binary);
                std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
                block_j = nlohmann::json::parse(contents);

                break;
            }
        }
    }
    
    return block_j;
}