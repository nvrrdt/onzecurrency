#include <string>
#include <map>
#include <stdlib.h>
#include <boost/process.hpp>

#include "auth.hpp"
#include "p2p.hpp"
#include "json.hpp"

#include "main.hpp"

using namespace Crowd;

int main(int argc, char *argv[])
{
    Auth a;
    std::map<std::string, std::string> cred = a.authentication();

    if (cred["error"] == "true")
    {
        std::cerr << "Error with authenticating" << std::endl;
        return 1;
    } else {
        P2p p;
        p.start_p2p(cred);             // get ip en start server .....

        // Tcp t;

        // t.server();

        // std::string message = "{'1': 'testology', '2': 'testology', '3': 'testology', '4': 'testology', '5': 'testology', '6': 'testology', '7': 'testology', '8': 'testology', '9': 'testology', '10': 'testology', '11': 'testology', '12': 'testology','13': 'testology', '14': 'testology', '15': 'testology', '16': 'testology', '17': 'testology', '18': 'testology', '19': 'testology', '20': 'testology', '21': 'testology', '22': 'testology', '23': 'testology', '24': 'testology', '25': 'testology', '26': 'testology', '27': 'testology', '28': 'testology', '29': 'testology', '30': 'testology', '31': 'testology', '32': 'testology', '33': 'testology', '34': 'testology', '35': 'testology', '36': 'testologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestologytestology'}";
        // t.client("", "", message, "");
        
        // nlohmann::json message_j;
        // message_j["req"] = "register";
        // message_j["id"] = "nvrrdt_to";
        
        // nlohmann::json message_j;
        // message_j["req"] = "connect";
        // message_j["id_from"] = "nvrrdt_from";
        // message_j["ip_from"] = "141.135.77.90";
        // message_j["id_to"] = "nvrrdt_to";
        // message_j["ip_to"] = "13.58.2.29";
        
        // t.client("", "", "", message_j.dump(), "");

        // dummy code:
        // merkle_tree mt;
        // mt.prep_block_creation();
    }

//     const std::string ip_mother_peer = "13.58.174.105"; // should be later taken from rocksdb or a pre-defined list
//     std::string ip_next_peer, new_peer;

//     namespace bp = boost::process;
//     bp::ipstream out;
//     bp::child c("curl -s https://api.ipify.org", bp::std_out > out); // TODO: should be replaced with a peer telling your ip adress
//     std::string my_public_ip;
//     out >> my_public_ip;
//     c.terminate();

// // Udp udp;
// // udp.udp_server();

//     std::cout << "server_peer: " << ip_mother_peer << ", my_ip: " << my_public_ip << std::endl;
//     if (ip_mother_peer == my_public_ip)
//     {
//         Upnp upnp;
//         Udp udp;
//         if(upnp.Upnp_main() == 0) // upnp possible
//         {
//             udp.udp_server();
//         }
//     }
    
//     if (cred["new_peer"] == "true")
//     {
//         nlohmann::json np = {{"new_peer", "true"}};
//         new_peer = np.dump();

//         Upnp upnp;
//         Udp udp;
//         // if(upnp.Upnp_main() == 0) // upnp possible
//         // {
//             ip_next_peer = udp.udp_client(ip_mother_peer, "None", new_peer);
//                             // send upnp_peer a request for ip_next_peer
//                             // download blockchain from ip_next_peer ---> in the future multiple peers should be chosen to download from
//                             // download rocksdb and verify with blockchain
//         // }
//     }
//     else
//     {
//         // lookup upnp_peer (ping) ip and next_peer (ping) ip in rocksdb --> ping because you don't know who is online, fallback hardcoded ip adress
//         // download blockchain from ip_next_peer ---> in the future multiple peers should be chosen to download from
//         // download rocksdb and verify with blockchain
//     }
    

    // lookup of ip next_peer if new_peer in a hardcoded list of a bootstrapping ip adress
    // if existing peer lookup ip next_peer in rocksdb
    // lookup next_upnp_peer to connect to previous next_peer
    // from previous next_peer communicate to layer 0 and so on

    // Protocol p;
    // p.hello_and_setup(cred["full_hash"]);
    // std::map<std::string, uint32_t> x = p.layer_management(cred["full_hash"]);


    return 0;
}