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
std::string Poco::Get(std::string key)
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
bool Poco::Put(std::string key, std::string value) // TODO: value must be json!!
{
    s = db->Put(rocksdb::WriteOptions(), key, value);
    if (s.ok()) return true;
    else return false;    
}
bool Poco::Delete(std::string key)
{
    rocksdb::Status s = db->Delete(rocksdb::WriteOptions(), key);
    if (s.ok()) return true;
    else return false;
}
std::string Poco::FindChosenOne(std::string key)
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
std::string Poco::FindNextPeer(std::string key)
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
std::string Poco::FindUpnpPeer(std::string key)
{
    std::string string_key_upnp_peer;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        if (it->key().ToString() >= key)
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

    return string_key_upnp_peer;
}
std::string Poco::FindNextUpnpPeer(std::string string_key)
{
    std::istringstream iss (string_key);
    uint32_t key;
    iss >> key;
    if (iss.fail())
    {
        std::cerr << "ERROR in creating the uint for the key!\n";
        return "1";
    }

    key++;

    std::string string_key_upnp_peer;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        if (it->key().ToString() >= std::to_string(key))
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

    std::stringstream ss;
    ss << key;

    return ss.str();
}
uint32_t Poco::TotalAmountOfPeers()
{
    std::string string_num;
    db->GetProperty("rocksdb.estimate-num-keys", &string_num);

    std::istringstream iss (string_num);
    uint32_t num;
    iss >> num;
    if (iss.fail())
    {
        std::cerr << "ERROR in creating the uint for the total_amount_of_peers!\n";
        return 1;
    }

    return num;
}