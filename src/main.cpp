#include "authentication.hpp"
#include "merkle_tree.hpp"
#include "p2p_handler.hpp"
#include "verification.hpp"

#include <future>
#include <thread>
#include <chrono>

using namespace crowd;
using namespace std;

int main()
{
    authentication auth;
    auth.auth();

    // Create a packaged_task using some task and get its future.
    std::packaged_task<void()> task1([] {
        p2p_handler ph;
        ph.p2p_switch();
    });

    std::packaged_task<void()> task2([] {
        merkle_tree mt;
        mt.prep_block_creation();
    });

    std::packaged_task<void()> task3([] {
        verification v;
        v.verification_handler();
    });

    // Run task on new thread.
    std::thread t1(std::move(task1));
    std::thread t2(std::move(task2));
    std::thread t3(std::move(task3));

    t1.join();
    t2.join();

    return 0;
}