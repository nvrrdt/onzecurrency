#include <future>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <vector>

#include "json.hpp"

#include "interface.hpp"
#include "p2p.hpp"
#include "poco.hpp"
#include "configdir.hpp"

using namespace Crowd;

std::string Protocol::latest_block()
{
    ConfigDir cd;
    std::string blockchain_folder_path = cd.GetConfigDir() + "blockchain";
    boost::system::error_code c;
    boost::filesystem::path path(blockchain_folder_path);

    std::string latest_block = "0";

    if (!boost::filesystem::exists(path))
    {
        return "no blockchain present in folder";
    }
    else
    {
        boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
        for ( boost::filesystem::directory_iterator itr( path ); itr != end_itr; ++itr )
        {
            if (itr->path().string() > latest_block)
            {
                latest_block = itr->path().string();
            }
        }
    }
    
    return latest_block; // TODO: also verify latest hash
}

std::map<uint32_t, uint256_t> Protocol::layers_management(uint256_t &amount_of_peers)
{
    const int nmax = 100;   // max number of peers per section
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
            // std::cout << std::endl << "max peers last layer: " << tlayer << std::endl;
            // std::cout << "max peers all layers: " << tmaxmax << std::endl;
            std::cout << std::endl << "amount of layers: " << maxreallayers << std::endl;
            std::cout << "amount_of_peers: " << amount_of_peers << std::endl;
            break;
        }
        else if (maxreallayers == imax && amount_of_peers > tmaxmax)
        {
            std::cerr << "ERROR: change imax as there are more peers than imax can handle" << std::endl;
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
        // std::cout << "quotient: " << quotient << std::endl;
        // std::cout << "remainder: " << remainder << std::endl;
    }
    else
    {
        quotient = overshoot / layercontents.at(maxreallayers-1);
        remainder = overshoot % layercontents.at(maxreallayers-1);
        // std::cout << remainder << " buckets in last layer with " << quotient+1 << " peers and " << (layercontents.at(maxreallayers-2))-remainder << " buckets with " << quotient << " peers." << std::endl;
        // std::cout << overshoot << " (overshoot) must be equal to " << (remainder*(quotient+1))+((layercontents.at(maxreallayers-2)-remainder)*quotient) << " (calculation of here before mentioned numbers)" << std::endl;
    }

    // std::cout << "numberlayers: " << maxreallayers << std::endl;
    // std::cout << "numbermaxbucketslastlayer: " << remainder << std::endl;
    // std::cout << "contentsmaxbucketslastlayer: " << quotient+1 << std::endl;
    // std::cout << "numberminbucketslastlayer: " << ((maxreallayers == 1) ? 100-remainder : (layercontents.at(maxreallayers-1)-remainder)) << std::endl;
    // std::cout << "contentsminbucketslastlayer: " << quotient << std::endl;
    // std::cout << "overshootlastlayer: " << overshoot << std::endl;

    // the peers between every top layer peer are counted and put in a map
    uint256_t counter, total_counter;
    std::map<uint32_t, uint256_t> chosen_ones_counter;

    for (int i = 1; i <= nmax; i++) // nmax = amoutn of chosen ones
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
        // std::cout << "coc: " << chosen_ones_counter[i] << std::endl;
        // total_counter += chosen_ones_counter[i];
    }
    std::cout << "amount of peers to verify: " << total_counter << std::endl;

    return chosen_ones_counter;
}

std::map<std::string, std::string> Protocol::partition_in_buckets(std::string &my_hash, std::string &next_hash)
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

    Poco poco;
    uint256_t count = poco.CountPeersFromTo(my_hash, next_hash);

    std::map<uint32_t, uint256_t> chosen_ones_counter = Protocol::layers_management(count);

    return Protocol::get_calculated_hashes(my_hash, chosen_ones_counter);
}

std::map<std::string, std::string> Protocol::get_calculated_hashes(std::string &my_hash, std::map<uint32_t, uint256_t> &chosen_ones_counter)
{
    // Get the hashes from rocksdb, the count is accessible now
    
    std::map<std::string, std::string> calculated_hashes;
    calculated_hashes[my_hash] = my_hash;
    return calculated_hashes;
}

std::string Protocol::get_blocks_from(std::string &latest_block_peer)
{
    nlohmann::json all_blocks_j;

    // put all blocks with block_nr in a string
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "blockchain"))
    {
        boost::filesystem::path p(cd.GetConfigDir() + "blockchain");

        typedef std::vector<boost::filesystem::path> vec;
        vec v;

        std::copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(v));

        std::sort(v.begin(), v.end()
            , [](boost::filesystem::path const& a, boost::filesystem::path const& b) {
            return std::stoi(a.filename().string()) < std::stoi(b.filename().string());
        });

        nlohmann::json block_j;
        for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
        {
            std::string str = it->stem().string();
            std::uint64_t value_this_blockchain_dir;
            std::istringstream iss(str.substr(6,18));
            iss >> value_this_blockchain_dir;
            std::uint64_t value_peer;
            if (latest_block_peer != "no blockchain present in folder")
            {
                std::istringstream isss(latest_block_peer);
                isss >> value_peer;
                std::cout << "value_this_blockchain_dir: " << value_this_blockchain_dir << " value_peer: " << value_peer << '\n';
                if (value_this_blockchain_dir > value_peer)
                {
                    std::ifstream stream(it->string(), std::ios::in | std::ios::binary);
                    std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
                    block_j["block_nr"] = value_this_blockchain_dir;
                    block_j["block"] = nlohmann::json::parse(contents);
                    all_blocks_j.push_back(block_j);
                }
            }
            else
            {
                std::ifstream stream(it->string(), std::ios::in | std::ios::binary);
                std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
                block_j["block_nr"] = value_this_blockchain_dir;
                block_j["block"] = nlohmann::json::parse(contents);
                all_blocks_j.push_back(block_j);
            }
        }
    }
    else
    {
        std::cout << "Blockchain directory doesn't exist!!" << std::endl;
    }

    return all_blocks_j.dump();
}

void Protocol::save_blocks_to_blockchain(std::string &msg)
{
    nlohmann::json json = nlohmann::json::parse(msg);
    std::cout << "msggggg: " << json.dump() << std::endl;
    std::string block_nr = json["block_nr"].get<std::string>();
    std::string block = json["block"].dump();

    ConfigDir cd;
    uint32_t first_chars = 11 - block_nr.length();
    std::string number = "";
    for (int i = 0; i <= first_chars; i++)
    {
        number.append("0");
    }
    number.append(block_nr);
    
    std::string block_file = "blockchain/block_" + number + ".json";
    std::cout << "blockfile: " << block_file << std::endl;

    std::string blockchain_folder_path = cd.GetConfigDir() + "blockchain";

    if (!boost::filesystem::exists(blockchain_folder_path))
    {
        boost::filesystem::create_directory(blockchain_folder_path);
    }

    cd.CreateFileInConfigDir(block_file, block);
}
