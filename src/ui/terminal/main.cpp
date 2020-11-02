#include <string>

#include "liblogin/auth.hpp"
#include "libcrowd/p2p.hpp"

#include "ui/terminal/main.hpp"

using namespace Crowd;

int main(int argc, char *argv[])
{
    auth a;
    a.authentication();

    std::string s = "1";
    std::string& ss = s;
    Protocol p;
    p.hello_and_setup(ss);

    return 0;
}