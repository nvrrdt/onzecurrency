#include "merkle_tree.hpp"
#include "p2p_handler.hpp"
#include "verification.hpp"
#include "external_ip.hpp"
#include "auth.hpp"

#include <future>
#include <thread>
#include <chrono>

using namespace crowd;

// GLOBAL Variables:
bool break_server_loop = false;

int main()
{
    auth a;
    a.authentication();

    std::packaged_task<void()> task1([] {
        verification v;
        v.verification_handler(::break_server_loop); // authentication and verification of the blocks
    });

    external_ip ei;
    ei.get_external_ip();

    // Create a packaged_task using some task and get its future.
    std::packaged_task<void()> task2([] {
        merkle_tree mt;
        mt.prep_block_creation(); // creation of the block2p
    });

    // Run task on new thread.
    std::thread t1(std::move(task1));
    std::thread t2(std::move(task2));

    t1.join();
    t2.join();

    return 0;
}