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
        virtual std::string Get(uint32_t key)
        {
            std::stringstream ss;
            ss << key;
            s = db->Get(leveldb::ReadOptions(), ss.str(), &value);
            if (s.ok())
            {
                return value;
            }
            else
            {
                return "";
            }
        }
        virtual int Put(uint32_t key, std::string value)
        {
            std::stringstream ss;
            ss << key;
            s = db->Put(leveldb::WriteOptions(), ss.str(), value);
            if (s.ok()) return 0;

            return 1;
        }
        virtual int Delete(uint32_t key)
        {
            std::stringstream ss;
            ss << key;
            leveldb::Status s = db->Delete(leveldb::WriteOptions(), ss.str());
            if (s.ok()) return 0;

            return 1;
        }
        virtual uint32_t FindChosenOne(uint32_t key)
        {
            std::stringstream ss;
            ss << key;

            leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
            for (it->Seek(ss.str()); it->Valid(); it->Next())
            {
                std::cout << "found chosen one: " << it->key().ToString() << std::endl;
            }

            std::istringstream iss (it->key().ToString());
            uint32_t key_real_chosen_one;
            iss >> key_real_chosen_one;
            if (iss.fail())
            {
                std::cerr << "ERROR in creating the uint for the key_real_chosen_one!\n";
                return 1;
            }

            return key_real_chosen_one;
        }
    };
}