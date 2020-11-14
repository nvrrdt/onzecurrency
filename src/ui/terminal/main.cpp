#include <string>

#include "auth.hpp"
#include "p2p.hpp"

#include "main.hpp"

using namespace Crowd;

int main(int argc, char *argv[])
{
    //Auth a;
    //a.authentication();

    //std::string s = "1";
    //std::string& ss = s;
    Protocol p;
    //p.hello_and_setup(ss);
    std::map<std::string, uint32_t> x = p.layer_management("1010102");
    printf("layers: %d\n", x["layers"]);
    printf("remainder: %d\n", x["remainder"]);
    printf("peersinremainder: %d\n", x["peersinremainder"]);
    printf("overshoot: %d\n", x["overshoot"]);

    return 0;
}