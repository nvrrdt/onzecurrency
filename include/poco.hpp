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
        leveldb::DB* db;
        leveldb::Status s;
        std::string value;
    public:
        Poco()
        {
            const char *homedir;
            if ((homedir = getenv("HOME")) == NULL) {
                homedir = getpwuid(getuid())->pw_dir;
            }
            std::string home, onlineusersdb;
            home = homedir;
            onlineusersdb = home  + "/.config/onzehub/onlineusersdb";

            leveldb::Options options;
            options.create_if_missing = true;
            s = leveldb::DB::Open(options, onlineusersdb, &db);
            //std::cout << "s == ok: " << s.ok() << " " << onlineusersdb << std::endl;
        }
        virtual std::string Get(uint32_t key) = 0;
        virtual int Put(uint32_t key, std::string value) = 0;
        virtual int Delete(uint32_t key) = 0;
        virtual uint32_t FindChosenOne(uint32_t key) = 0;
    };
}