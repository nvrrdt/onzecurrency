#include <iostream>
#include <map>
#include <enet/enet.h>

namespace Crowd
{
    class AllFullHashes
    {
    public:
        static void add_to_all_full_hashes(enet_uint32 participant, std::string full_hash_req);
        static std::map<enet_uint32, std::string> get_all_full_hashes();
        static void reset_all_full_hashes();
    private:
        static std::map<enet_uint32, std::string> all_full_hashes_;
    };
}
