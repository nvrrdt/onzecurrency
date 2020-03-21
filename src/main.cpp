#include "merkle_tree.hpp"
#include "p2p_handler.hpp"
#include "verification.hpp"
#include "external_ip.hpp"

#include <future>
#include <thread>
#include <chrono>

using namespace crowd;
using namespace std;

int main()
{
    verification v;
    v.verification_handler(); // authentication and verification of the blocks

    external_ip ei;
    ei.get_external_ip();

    // Create a packaged_task using some task and get its future.
    std::packaged_task<void()> task1([] {
        p2p_handler ph;
        ph.p2p_switch(); // handling of the p2p
    });

    std::packaged_task<void()> task2([] {
        merkle_tree mt;
        mt.prep_block_creation(); // creation of the block
    });

    // Run task on new thread.
    std::thread t1(std::move(task1));
    std::thread t2(std::move(task2));

    t1.join();
    t2.join();

    return 0;
}