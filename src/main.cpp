#include "authentication.hpp"
#include "merkle_tree.hpp"

using namespace crowd;
using namespace std;

int main()
{
    authentication a;
    a.auth();

    merkle_tree mt;
    mt.do_something();

    return 0;
}