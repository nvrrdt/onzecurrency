#include <string>
#include <map>

#include "auth.hpp"
#include "p2p.hpp"

#include "main.hpp"

using namespace Crowd;

int main(int argc, char *argv[])
{
    Auth a;
    std::map<std::string, std::string> cred = a.authentication();

    if (cred["error"] == "true") return 1;

    Protocol p;
    p.hello_and_setup(cred["full_hash"]);
    std::map<std::string, uint32_t> x = p.layer_management(cred["full_hash"]);


    return 0;
}