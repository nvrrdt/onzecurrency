#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "leveldb/db.h"

namespace Crowd
{
    /**
     * Poco is the system that enables getting, putting and deleting entries in leveldb
     * and also finding the chosen_one in that database.
     * The chosen_one is the user who collects the data from the verifiers
     * and broadcasts the success of block creation to all the online users.
     */

    class Poco
    {
    private:
        leveldb::DB* db;
        leveldb::Status s;
        std::string value;
    public:
        Poco();
        std::string Get(uint32_t key);
        bool Put(uint32_t key, std::string value);
        bool Delete(uint32_t key);
        uint32_t FindChosenOne(uint32_t key);
    };
}