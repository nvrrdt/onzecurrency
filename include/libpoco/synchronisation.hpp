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
            delete bm_crowd_;
            delete bm_coin_;
        }
        void get_sleep_and_create_block();

        std::string get_genesis_datetime();

        static std::string get_datetime_now();
    private:
        void get_sleep_until();
        static void set_datetime_now(std::string datetime);
    private:
        IntroMsgMap intro_msg_map_;
        IpHEmail ip_hemail_vec_;
        
        PocoCrowd pococr_;
        PocoCoin pococo_;
        Transactions tx_;
        BlockMatrix *bm_crowd_ = new Poco::BlockMatrix();
        BlockMatrixC *bm_coin_ = new Poco::BlockMatrixC();

        static std::string datetime_;
    };
}