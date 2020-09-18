#include <onzehub/auth.hpp>
#include <onzehub/p2p.hpp>

#include "main.hpp"

using namespace crowd;

int main(int argc, char *argv[])
{
    auth a;
    a.authentication();

    p2p_handler ph;
    ph.p2p_switch("download", "172.31.24.198");

    return 0;
}