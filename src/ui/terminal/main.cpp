#include <string>

#include "auth.hpp"
#include "p2p.hpp"

#include "main.hpp"

using namespace Crowd;

int main(int argc, char *argv[])
{
    Auth a;
    a.authentication();

    std::string s = "1";
    std::string& ss = s;
    Protocol p;
    p.hello_and_setup(ss);

    return 0;
}