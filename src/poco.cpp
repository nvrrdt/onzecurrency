#include "poco.hpp"

using namespace Crowd;

std::string Poco::Get(uint32_t key)
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

int Poco::Put(uint32_t key, std::string value)
{
    std::stringstream ss;
    ss << key;
    s = db->Put(leveldb::WriteOptions(), ss.str(), value);
    if (s.ok()) return 0;

    return 1;
}

int Poco::Delete(uint32_t key)
{
    std::stringstream ss;
    ss << key;
    leveldb::Status s = db->Delete(leveldb::WriteOptions(), ss.str());
    if (s.ok()) return 0;

    return 1;
}

uint32_t Poco::FindChosenOne(uint32_t key)
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