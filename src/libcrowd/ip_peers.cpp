#include "ip_peers.hpp"

using namespace Crowd;

IpPeers::IpPeers()
{
    // if exists, read ip_peers.json, if not exists create with ip_mother_peer
    ConfigDir cd;
    if (!boost::filesystem::exists(cd.GetConfigDir() + "ip_peers.json"))
    {
        // not existant: create json
        nlohmann::json json;
        json["_comment"] = "DON'T MODIFY THIS FILE YOURSELF";
        json["ip_list"].push_back("13.58.174.105");
        std::string jd = json.dump();
        std::string file = "ip_peers.json";
        cd.CreateFileInConfigDir(file, jd);

        ip_s_ = json["ip_list"].get<std::vector<std::string>>();
    }
    else
    {
        // existant: read list
        std::fstream ip_peers;
        ip_peers.open(cd.GetConfigDir() + "ip_peers.json", std::ios::in);
        std::string full_str;
        if (ip_peers.is_open())
        {
            std::string str;
            while(getline(ip_peers, str))
            {
                std::cout << str << std::endl;
                full_str += str;
            }
            ip_peers.close(); //close the file object.
        }

        nlohmann::json json;
        json = nlohmann::json::parse(full_str);

        ip_s_ = json["ip_list"].get<std::vector<std::string>>();
    }
}