#include "poco.hpp"

using namespace Crowd;

Poco::Poco()
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
std::string Poco::Get(uint32_t key)
{
    std::stringstream ss;
    ss << key;
    s = db->Get(leveldb::ReadOptions(), ss.str(), &value);
    if (s.ok())
    {
        if (value == "test") return ""; // A hack, somehow leveldb's value is test when there is no entry
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

    std::string string_key_real_chosen_one;

    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    for (it->Seek(ss.str()); it->Valid(); it->Next())
    {
        string_key_real_chosen_one = it->key().ToString();
    }

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