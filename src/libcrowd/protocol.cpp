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

/**
 * - version protocol
 * - download blockchain
 * - introduction new user
 * - hello --> online presence
 * - goodbye --> leaving the network
 * - ...
 * 
 * - only calculated users can access the peer, otherwise the upnp provider refuses
 * - lookup peer in map of users
 * - then go to its upnp provider if its upnp isn't enabled
 * - let the protocol work
 * 
 * - a nonce must be used for calculating the chosen_one
 * - otherwise there exists DDOS'ing
 * - if no nonce, the chosen_one is known, and its 'desciples' are also known
 * 
 * - hello: - lookup upnp provider above your own hash, get connected, if upnp provider offline lookup next until one online,
 *            this one gives you correct upnp provider
 *          - then update your map by asking the peer above you
 *          - then communicate to the whole network by which you divide the whole network by x divided by x,
 *            the corresponding users above the calculated hash will communicate to the whole network
 * - goodbye: - communicate to the whole network by which you divide the whole network by x divided by x,
 *              the corresponding users above the calculated hash will communicate to the whole network
 * - version: - just ask the version as first request
 * - download: - download the whole blockchain from 13 users who are the hashes of your hash
 *               or download the latest blocks from 13 users who are the hashes of your hash
 * - new_user: - just a few hardcoded and upnp enabled peers will are used for this case
 *             - the hardcoded peer above the hash of the new user will communicate the new user to all the peers
 * 
 * The json message might contain:
 * - full hash = fh [uint32]
 * - hash of email = em [uint32]
 * - salt = sa [uint32]
 * - version = ve [major.minor.micro]
 * - ip adress = ip [ip]
 * - full node = fn [true | false]
 * - upnp = up [true | false]
 * !! last login datetime must be in block
 * 
 * A block in the blockchain must contain:
 * - full hash = fh [uint32] of every peer in basket
 * - hash of email = em [uint32] of every peer in basket
 * - full hash = sa [uint32] of every peer in basket
 * - timestamp = ts [datetime]
 * - previous hash = ph [uint32]
 * - counter value = co [uint]
 * - ...
 */

// - hello network!
// - verification of blockchain
// - new user in network
// - goodbye network

int Protocol::hello_and_setup(std::string& my_user_login_hash)
{
    /**
     * - search your upnp provider and say hello to the network
     */

    Poco poco;
    std::string upnp_peer_key = poco.FindUpnpPeer(my_user_login_hash);
    nlohmann::json upnp_peer_value = nlohmann::json::parse(poco.Get(upnp_peer_key));

    nlohmann::json msg = {{"version", 1.0}, {"hash_of_new_peer", my_user_login_hash}, {"fullnode", true}};

    Upnp upnp;
    Udp udp;
    if(upnp.Upnp_main() == 0) // upnp possible
    {
        msg["upnp"] = true;

        if (udp.udp_client(upnp_peer_value["ip"].dump(), "None", msg.dump()) == 0)
        {
            // set as upnp server in thread
            std::packaged_task<void()> task1([&] {
                Udp udp;
                udp.udp_server();
            });
        
            // Run task on new thread.
            std::thread t1(std::move(task1));

            t1.join();
        }
        else
        {
            // find next upnp provider in rocksdb and set as new upnp server in thread
            while (true)
            {
                std::string next_upnp_peer_key = poco.FindNextUpnpPeer(upnp_peer_key);
                nlohmann::json next_upnp_peer_value = nlohmann::json::parse(poco.Get(upnp_peer_key));

                if (udp.udp_client(next_upnp_peer_value["ip"].dump(), "None", "helloupnpenabled") == 0)
                {
                    // set as upnp server in thread
                    std::packaged_task<void()> task1([&] {
                        Udp udp;
                        udp.udp_server();
                    });
                
                    // Run task on new thread.
                    std::thread t1(std::move(task1));

                    t1.join();

                    break;
                }
                else
                {
                    upnp_peer_key = next_upnp_peer_key;
                }
                // TODO: create fallback for when no upnp peer is found, foresee a hardcoded fallback
            }
        }
        
    }
    else // upnp not possible
    {
        msg["upnp"] = false;

        if (udp.udp_client(upnp_peer_value["ip"].dump(), "None", "helloupnpdisabled") == true)
        {
            // set as upnp client in thread
            std::packaged_task<void()> task1([&] {
                Udp udp;
                udp.udp_client(upnp_peer_value["ip"].dump(), "None", "wait");
            });
        
            // Run task on new thread.
            std::thread t1(std::move(task1));

            t1.join();
        }
        else
        {
            // find next upnp provider in rocksdb and set as upnp client in thread
            while (true)
            {
                std::string next_upnp_peer_key = poco.FindNextUpnpPeer(upnp_peer_key);
                nlohmann::json next_upnp_peer_value = nlohmann::json::parse(poco.Get(upnp_peer_key));

                if (udp.udp_client(next_upnp_peer_value["ip"].dump(), "None", "helloupnpenabled") == 0)
                {
                    // set as upnp server in thread
                    std::packaged_task<void()> task1([&] {
                        Udp udp;
                        udp.udp_client(upnp_peer_value["ip"].dump(), "None", "wait");
                    });
                
                    // Run task on new thread.
                    std::thread t1(std::move(task1));

                    t1.join();

                    break;
                }
                else
                {
                    upnp_peer_key = next_upnp_peer_key;
                }
                // TODO: create fallback for when no upnp peer is found, foresee a hardcoded fallback
            }
        }
    }
    
    return 0;
}

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

// int Protocol::verify_latest_block(std::string latest_block_peer)
// {
//     /**
//      * Verify that the latest block response (response_hello) from the peer equals your latest block
//      * If not, download all the latest blocks from FindNextPeer(your_hash)
//      * If too many blocks then request more peers
//      */

//     ConfigDir cd;
//     std::string blockchain_folder_path = cd.GetConfigDir() + "blockchain";
//     boost::system::error_code c;
//     boost::filesystem::path path(blockchain_folder_path);

//     std::string latest_block = "0";

//     if (!boost::filesystem::exists(path))
//     {
//         std::cout << "no blockchain present in folder" << std::endl;
//         return 1;
//     }
//     else
//     {
//         boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
//         for ( boost::filesystem::directory_iterator itr( path ); itr != end_itr; ++itr )
//         {
//             if (itr->path().string() > latest_block)
//             {
//                 latest_block = itr->path().string();
//             }
//         }

//         if (latest_block_peer > latest_block)
//         {
//             // Download or update blockchain, ask peer via FindNextPeer(key)
//             Poco p;
//             uint32_t next_peer = p.FindNextPeer(/*my_key from liblogin*/ 1);
//             // send next_peer message to download/update blockchain
//             return 0;
//         }
//         else if (latest_block_peer == latest_block)
//         {
//             // Should be latest block
//             return 0;
//         }
//         else /* latest_block_peer < latest_block */
//         {
//             std::cout << "Something went wrong, you're blockchain is more to date then your peer" << std::endl;
//             // TODO: ping other peer to know the latest block

//             return 1; // temporary
//         }
        
//     }
// }

// int Protocol::communicate_to_all(boost::array<char, 128> msg)
// {
//     /**
//      * Partition rocksdb (size = uint32_t) in equal partitions
//      * Let the online peers know the update
//      * Roundup(amount_of_online_peers^(1/3)) or Roundup(100^(1/3)) = Roundup(~4.6) = 5 partitions and Roundup(4.1) = 5 partitions
//      * It is known who may signal you!
//      */

//     // #include <limits>
//     // std::numeric_limits<uint32_t>::max(); // should be 2^32!

//     // #include <math.h>
//     // ceil(2.3)
//     // pow (7.0, 3.0)

//     // find all peers to communciate to ...

//     return 0;
// }

std::map<std::string, uint32_t> Protocol::layer_management(std::string string_total_amount_of_peers)
{
    const int nmax = 100;   // max number of peers per section
    uint32_t i;                  // number of layers
    uint64_t tmaxmax = 0, tmaxmin = 0, overshoot = 0, tlayer = 0;
    uint32_t total_amount_of_peers = static_cast<uint32_t>(std::stoul(string_total_amount_of_peers)); // real number of peers

    std::vector<uint64_t> layercontents;

    for (i = 1; i <= 20; i++)
    {
        
        tlayer = pow(nmax, i);  // max number of peers in a layer
        tmaxmax += tlayer;      // max number of peers in all layers

        layercontents.push_back(tlayer);

        if (total_amount_of_peers <= tmaxmax)
        {
            printf("t(last)layer: %ld and tmaxmax: %ld\n", tlayer, tmaxmax);
            printf("total_amount_of_peers: %d\n", total_amount_of_peers);
            printf("layers i: %d\n", i);
            break;
        }
    }

    tmaxmin = tmaxmax - layercontents.at(i-1);      // max number of peers in all layers minus last layer
    printf("tmaxmin: %ld\n", tmaxmin);
    overshoot = total_amount_of_peers - tmaxmin;    // number of real peers in last layer
    printf("overshoot: %ld\n", overshoot);
    
    // partitioning of last layer:
    // explantation: remainder peers (layer before last) connect to (quotient + 1) in last layer
    //               (layercont - remainder) peers (layer before last) connect to (quotient) in last layer
    uint32_t quotient, remainder;
    if (i == 1) // only 1 layer == layer 0
    {
        quotient = 0;
        remainder = overshoot;
        printf("quotient: %d\n", quotient);
    }
    else
    {
        quotient = total_amount_of_peers / layercontents.at(i-2);
        remainder = total_amount_of_peers % layercontents.at(i-2);
        printf("quotient: %d and remainder: %d and layercont: %ld\n", quotient, remainder, layercontents.at(i-2));
    }

    std::map<std::string, uint32_t> result;
    result["layers"] = i;
    result["remainder"] = remainder;
    result["peersinremainder"] = quotient+1;
    result["overshoot"] = overshoot;

    printf("\nlayers: %d\n", result["layers"]);
    printf("remainder: %d\n", result["remainder"]);
    printf("peersinremainder: %d\n", result["peersinremainder"]);
    printf("overshoot: %d\n", result["overshoot"]);

    return result;
}
