#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision;

#include "configdir.hpp"

#include "rocksdb/db.h"
#include <array>

namespace Crowd
{
    /**
     * Rocksy is the system that enables getting, putting and deleting entries in rocksdb
     * and also finding the chosen_one in that database.
     */

    class Rocksy
    {
    private:
        rocksdb::DB* db;
        rocksdb::Status s;
        std::string value;
    protected:
        ConfigDir cd;
        std::string usersdb_folder_path = cd.GetConfigDir() + "usersdb";
        std::string transactionsdb_folder_path = cd.GetConfigDir() + "transactionsdb";
    public:
        Rocksy(std::string which_db);
        ~Rocksy();
        std::string Get(std::string &key);
        bool Put(std::string &key, std::string &value);
        bool Delete(std::string &key);
        std::string FindChosenOne(std::string &key);
        std::string FindNextPeer(std::string &key);
        std::string FindServerPeer(std::string &key);
        std::string FindNextServerPeer(std::string &key);
        uint256_t TotalAmountOfPeers();
        uint256_t CountPeersFromTo(std::string &my_hash, std::string &next_hash);
        std::string FindPeerFromTillCount(std::string &key, uint256_t &count);
        void DatabaseDump();
    };
}