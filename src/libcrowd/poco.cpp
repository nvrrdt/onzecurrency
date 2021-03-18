#include "json.hpp"

#include "poco.hpp"

using namespace Crowd;

Poco::Poco()
{
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status s = rocksdb::DB::Open(options, usersdb_folder_path, &db);
    std::cout << "s == ok: " << s.ok() << " : " << s.ToString() << " : " << usersdb_folder_path << std::endl;
}
Poco::~Poco()
{
    delete db;
}
std::string Poco::Get(std::string &key)
{
    s = db->Get(rocksdb::ReadOptions(), key, &value);
    if (s.ok())
    {
        if (value == "test") return ""; // A hack, somehow rocksdb's value is 'test' when there is no entry
        return value;
    }
    else
    {
        return "";
    }
}
bool Poco::Put(std::string &key, std::string &value) // TODO: value must be json!!
{
    s = db->Put(rocksdb::WriteOptions(), key, value);
    if (s.ok()) return true;
    else return false;    
}
bool Poco::Delete(std::string &key)
{
    rocksdb::Status s = db->Delete(rocksdb::WriteOptions(), key);
    if (s.ok()) return true;
    else return false;
}
std::string Poco::FindChosenOne(std::string &key)
{
    std::string string_key_real_chosen_one;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        if (it->key().ToString() >= key)
        {
            string_key_real_chosen_one = it->key().ToString();
            break;
        }
        else
        {
            for (it->SeekToFirst(); it->Valid(); it->Next())
            {
                string_key_real_chosen_one = it->key().ToString();
                break;
            }
            break;
        }
    }
    delete it;

    return string_key_real_chosen_one;
}
std::string Poco::FindNextPeer(std::string &key)
{
    std::string string_key_next_peer;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        std::cout << it->key().ToString() << ": "  << it->value().ToString() << std::endl;
        if (it->key().ToString() > key)
        {
            string_key_next_peer = it->key().ToString();
            break;
        }
        else
        {
            // if next peer is the first in whole level db, go search from start
        }
    }
    delete it;

    return string_key_next_peer;
}
std::string Poco::FindServerPeer(std::string &key)
{
    std::string string_key_server_peer;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        if (it->key().ToString() >= key)
        {
            std::cout << "it: " << it->value().ToString() << std::endl;
            // Search in it->value for server enabledness
            nlohmann::json j = nlohmann::json::parse(it->value().ToString());
            if (j["server"] == true)
            {
                string_key_server_peer = it->key().ToString();
                break;
            }
        }
        else if (false)
        {
            // TODO: what if you are at the and of rocksdb and need to restart searching from the beginning
            // see FindChosenOne()
        }
    }
    delete it;

    return string_key_server_peer;
}
std::string Poco::FindNextServerPeer(std::string &string_key)
{
    std::istringstream iss (string_key);
    uint256_t key;
    iss >> key;
    if (iss.fail())
    {
        std::cerr << "ERROR in creating the uint for the key!\n";
        return "1";
    }

    key++;

    std::string string_key_server_peer;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        if (it->key().ToString() >= string_key)
        {
            // Search in it->value for next one's server enabledness
            nlohmann::json j = nlohmann::json::parse(it->value().ToString());
            if (j["server"] == true)
            {
                string_key_server_peer = it->key().ToString();
                break;
            }
        }
        else if (false)
        {
            // TODO: what if you are at the and of rocksdb and need to restart searching from the beginning
            // see FindChosenOne()
        }
    }
    delete it;

    return string_key_server_peer;
}
uint256_t Poco::TotalAmountOfPeers() // TODO: shouldn't uint256_t should be used?
{
    std::string string_num;
    db->GetProperty("rocksdb.estimate-num-keys", &string_num);

    std::istringstream iss (string_num);
    uint256_t num;
    iss >> num;
    if (iss.fail())
    {
        std::cerr << "ERROR in creating the uint for the total_amount_of_peers!\n";
        return 1;
    }

    return num;
}

uint256_t Poco::CountPeersFromTo(std::string &from, std::string &to)
{
    uint256_t count = 0;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    if (from < to)
    {
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            if (it->key().ToString() > from)
            {
                std::cout << "it->key(): " << it->key().ToString() << std::endl;
                count++;

                if (it->key().ToString() >= to)
                {
                    break;
                }
            }
        }
    }
    else if (from == to)
    {
        std::string last;
        for (it->SeekToLast(); it->Valid(); it->Prev())
        {
            last = it->key().ToString();
            break;
        }

        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            if (it->key().ToString() > from)
            {
                count++;

                if (it->key().ToString() == last)
                {
                    for (it->SeekToFirst(); it->Valid(); it->Next())
                    {
                        count++;

                        if (it->key().ToString() >= to)
                        {
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }
    else
    {
        // Shouldn't happen!!
        std::cerr << "ERROR: to should be >= to from" << std::endl;
    }

    delete it;

    return count;
}