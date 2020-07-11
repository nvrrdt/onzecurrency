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
std::string Poco::Get(uint32_t key)
{
    std::stringstream ss;
    ss << key;
    s = db->Get(rocksdb::ReadOptions(), ss.str(), &value);
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
bool Poco::Put(uint32_t key, std::string value) // TODO: value must be json!!
{
    std::stringstream ss;
    ss << key;
    s = db->Put(rocksdb::WriteOptions(), ss.str(), value);
    if (s.ok()) return true;
    else return false;    
}
bool Poco::Delete(uint32_t key)
{
    std::stringstream ss;
    ss << key;
    rocksdb::Status s = db->Delete(rocksdb::WriteOptions(), ss.str());
    if (s.ok()) return true;
    else return false;
}
uint32_t Poco::FindChosenOne(uint32_t key)
{
    std::stringstream ss;
    ss << key;

    std::string string_key_real_chosen_one;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        if (it->key().ToString() >= ss.str())
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

    std::istringstream iss (string_key_real_chosen_one);
    uint32_t key_real_chosen_one;
    iss >> key_real_chosen_one;
    if (iss.fail())
    {
        std::cerr << "ERROR in creating the uint for the key_real_chosen_one!\n";
        return 1;
    }

    return key_real_chosen_one;
}
uint32_t Poco::FindNextPeer(uint32_t key)
{
    std::stringstream ss;
    ss << key;

    std::string string_key_next_peer;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        std::cout << it->key().ToString() << ": "  << it->value().ToString() << std::endl;
        if (it->key().ToString() > ss.str())
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

    std::istringstream iss (string_key_next_peer);
    uint32_t key_next_peer;
    iss >> key_next_peer;
    if (iss.fail())
    {
        std::cerr << "ERROR in creating the uint for the key_next_peer!\n";
        return 1;
    }

    return key_next_peer;
}
uint32_t Poco::FindUpnpPeer(uint32_t key)
{
    std::stringstream ss;
    ss << (key);

    std::string string_key_upnp_peer;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        if (it->key().ToString() >= ss.str())
        {
            // Search in it->value for upnp enabledness
            nlohmann::json j = nlohmann::json::parse(it->value().ToString());
            if (j["upnp"] == true)
            {
                string_key_upnp_peer = it->key().ToString();
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

    std::istringstream iss (string_key_upnp_peer);
    uint32_t key_upnp_peer;
    iss >> key_upnp_peer;
    if (iss.fail())
    {
        std::cerr << "ERROR in creating the uint for the key_upnp_peer!\n";
        return 1;
    }

    return key_upnp_peer;
}
uint32_t Poco::FindNextUpnpPeer(uint32_t key)
{
    std::stringstream ss;
    ss << (key + 1);

    std::string string_key_upnp_peer;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        if (it->key().ToString() >= ss.str())
        {
            // Search in it->value for next one's upnp enabledness
            nlohmann::json j = nlohmann::json::parse(it->value().ToString());
            if (j["upnp"] == true)
            {
                string_key_upnp_peer = it->key().ToString();
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

    std::istringstream iss (string_key_upnp_peer);
    uint32_t key_upnp_peer;
    iss >> key_upnp_peer;
    if (iss.fail())
    {
        std::cerr << "ERROR in creating the uint for the key_upnp_peer!\n";
        return 1;
    }

    return key_upnp_peer;
}