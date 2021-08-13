#pragma once

#include <stdio.h>
#include <iostream>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <ctime>
#include <unistd.h>

#include "poco_crowd.hpp"
#include "poco_coin.hpp"
#include "transactions.hpp"
#include "block_matrix.hpp"
#include "p2p.hpp"
#include "configdir.hpp"
#include "intro_msg_mat.hpp"
#include "all_hashes_mat.hpp"

namespace Poco
{
    class Synchronisation
    {
    public:
        ~Synchronisation()
        {
            delete bm_;
            delete bmc_;
        }
        void get_sleep_and_create_block();

        static bool get_break_block_creation_loops()
        {
            return break_block_creation_loops_;
        }

        std::string get_latest_datetime();

        static void set_break_block_creation_loops(bool break_loops)
        {
            break_block_creation_loops_ = break_loops;
        }
    private:
        void get_sleep_until();
    private:
        IntroMsgVec intro_msg_vec_;
        IpHEmail ip_hemail_vec_;
        
        PocoCrowd pococr_;
        PocoCoin pococo_;
        Transactions tx_;
        BlockMatrix *bm_ = new Poco::BlockMatrix();
        BlockMatrixC *bmc_ = new Poco::BlockMatrixC();

        static bool break_block_creation_loops_;
    };
}