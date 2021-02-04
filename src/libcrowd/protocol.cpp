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

std::map<std::string, uint64_t> Protocol::layers_management(std::string &string_total_amount_of_peers)
{
    const int nmax = 100;   // max number of peers per section
    uint32_t i;                  // number of layers
    uint64_t tmaxmax = 0, tmaxmin = 0, overshoot = 0, tlayer = 0;
    uint64_t total_amount_of_peers = static_cast<uint64_t>(std::stoul(string_total_amount_of_peers)); // real number of peers

    std::vector<uint64_t> layercontents;

    int imax = 64;
    for (i = 1; i <= imax; i++)
    {
        
        tlayer = pow(nmax, i);  // max number of peers in a layer
        tmaxmax += tlayer;      // max number of peers in all layers

        layercontents.push_back(tlayer);

        if (total_amount_of_peers <= tmaxmax)
        {
            printf("\nmax peers last layer: %ld, and max peers all layers: %ld\n", tlayer, tmaxmax);
            printf("total_amount_of_peers: %ld\n", total_amount_of_peers);
            printf("layers i: %d\n", i);
            break;
        }
        else if (i == imax && total_amount_of_peers > tmaxmax)
        {
            std::cerr << "ERROR: change imax as there are more peers than imax can handle" << std::endl;
        }
    }

    tmaxmin = tmaxmax - layercontents.at(i-1);      // max number of peers in all layers minus last layer
    overshoot = total_amount_of_peers - tmaxmin;    // number of peers in last layer
    
    // partitioning of last layer:
    // explanation: (remainder) contains (quotient + 1) peers in 1 bucket of last layer
    //              (layercontents(last_layer) - remainder) contains (quotient) peers in 1 bucket of last layer
    //              (layercontents(last_layer)) is maximum number of peers in that layer
    uint64_t quotient, remainder;
    if (i == 1) // only 1 layer == layer 0
    {
        quotient = 0;
        remainder = overshoot;
        printf("quotient: %ld\n", quotient);
        printf("remainder: %ld\n", remainder);
    }
    else
    {
        quotient = overshoot / layercontents.at(i-1);
        remainder = overshoot % layercontents.at(i-1);
        std::cout << remainder << " buckets with " << quotient+1 << " peers and " << (layercontents.at(i-2))-remainder << " buckets with " << quotient << " peers." << std::endl;
        std::cout << overshoot << " (overshoot) must be equal to " << (remainder*(quotient+1))+((layercontents.at(i-2)-remainder)*quotient) << " (calculation of here before mentioned numbers)" << std::endl;
    }

    std::map<std::string, uint64_t> result;
    result["numberlayers"] = i;
    result["numbermaxbucketslastlayer"] = remainder;
    result["contentsmaxbucketslastlayer"] = quotient+1;
    result["numberminbucketslastlayer"] = ((i == 1) ? 100-remainder : layercontents.at(i-1)-remainder);
    result["contentsminbucketslastlayer"] = quotient;
    result["overshootlastlayer"] = overshoot;

    std::cout << "numberlayers: " << i << std::endl;
    std::cout << "numbermaxbucketslastlayer: " << remainder << std::endl;
    std::cout << "contentsmaxbucketslastlayer: " << quotient+1 << std::endl;
    std::cout << "numberminbucketslastlayer: " << ((i == 1) ? 100-remainder : (layercontents.at(i-1)-remainder)) << std::endl;
    std::cout << "contentsminbucketslastlayer: " << quotient << std::endl;
    std::cout << "overshootlastlayer: " << overshoot << std::endl;

    return result;
}
