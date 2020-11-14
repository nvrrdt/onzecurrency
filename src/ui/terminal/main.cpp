#include <string>

#include "auth.hpp"
#include "p2p.hpp"

#include "main.hpp"

using namespace Crowd;

int main(int argc, char *argv[])
{
    Auth a;
    std::string full_hash = a.authentication();

    Protocol p;
    p.hello_and_setup(full_hash);
    std::map<std::string, uint32_t> x = p.layer_management(full_hash);


    return 0;
}