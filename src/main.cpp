#include "authentication.hpp"
#include "merkle_tree.hpp"
#include "p2p_handler.hpp"

using namespace crowd;
using namespace std;

int main()
{
    authentication auth;
    auth.auth();

    p2p_handler ph;
    ph.p2p_switch();

    return 0;
}