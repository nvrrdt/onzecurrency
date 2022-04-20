#include <math.h>
#include <limits>
#include <algorithm>

#include "json.hpp"

#include "rocksy.hpp"

#include "print_or_log.hpp"

using namespace Crowd;

Rocksy::Rocksy(std::string which_db)
{
    Common::Print_or_log pl;

    rocksdb::Options options;
    options.create_if_missing = true;
    if (which_db == "usersdb")
    {
        rocksdb::Status s = rocksdb::DB::Open(options, usersdb_folder_path, &db);
        pl.handle_print_or_log({"s == ok:", std::to_string(s.ok()), ":", s.ToString(), ":", usersdb_folder_path});
    }
    else if (which_db == "usersdbreadonly")
    {
        rocksdb::Status s = rocksdb::DB::OpenForReadOnly(options, usersdb_folder_path, &db);
        pl.handle_print_or_log({"s == ok:", std::to_string(s.ok()), ":", s.ToString(), ":", usersdb_folder_path});
    }
    else if (which_db == "transactionsdb")
    {
        rocksdb::Status s = rocksdb::DB::Open(options, transactionsdb_folder_path, &db);
        pl.handle_print_or_log({"s == ok:", std::to_string(s.ok()), ":", s.ToString(), ":", transactionsdb_folder_path});
    }
    else if (which_db == "transactionsdbradonly")
    {
        rocksdb::Status s = rocksdb::DB::OpenForReadOnly(options, transactionsdb_folder_path, &db);
        pl.handle_print_or_log({"s == ok:", std::to_string(s.ok()), ":", s.ToString(), ":", transactionsdb_folder_path});
    }
}

Rocksy::~Rocksy()
{
    delete db;
}

std::string Rocksy::Get(std::string &key)
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

bool Rocksy::Put(std::string &key, std::string &value) // TODO: value must be json!!
{
    s = db->Put(rocksdb::WriteOptions(), key, value);
    if (s.ok()) return true;
    else return false;    
}

bool Rocksy::Delete(std::string &key)
{
    rocksdb::Status s = db->Delete(rocksdb::WriteOptions(), key);
    if (s.ok()) return true;
    else return false;
}

std::string Rocksy::FindCoordinator(std::string &user_id, std::string &hash_data)
{
    /**
     * - find shard
     * - hash_block % (fair_)user_id_in_shard[i] --> 1 with lowest remainder is coordinator
     * 
     */

    Poco::DatabaseSharding ds;
    auto shard_users = ds.get_shard_users(user_id);
    
    std::string coordinator;
    uint256_t lowest = std::numeric_limits<uint256_t>::max(), remainder;
    for (auto& user: shard_users)
    {
        if (user >= hash_data)
        {
            remainder = user % hash_data;
            if (remainder < lowest)
            {
                lowest = remainder;
                coordinator = user;
            }
        }
        else
        {
            remainder = hash_data % user;
            if (remainder < lowest)
            {
                lowest = remainder;
                coordinator = user;
            }
        }
    }

    return coordinator;
}

std::vector<std::string> Rocksy::FindChosenOnes(std::string &user_id, std::string &hash_data)
{
    /**
     * - find shard
     * - hash_block % (fair_)user_id_in_shard[i] --> max 128 with lowest remainder are chosen_ones
     * 
     */

    Poco::DatabaseSharding ds;
    auto shard_users = ds.get_shard_users(user_id);

    std::vector<std::string> chosen_ones;
    std::vector<std::pair<uint256_t, std::string>> preps;
    uint256_t remainder;
    for (auto& user: shard_users)
    {
        if (user >= hash_data)
        {
            remainder = user % hash_data;
        }
        else
        {
            remainder = hash_data % user;
        }

        std::pair<uint256_t, std::string> pair;
        pair.first = remainder;
        pair.second = user;
        preps.push_back(pair);
    }

    std::sort (preps.begin(), preps.end());
    for (uint64_t i = 0; i < preps.size(); i++)
    {
        chosen_ones.push_back(preps.at(i).second);

        if (chosen_ones.size() > 128) break;
    }

    return chosen_ones;
}

std::string Rocksy::FindNextPeer(std::string &key)
{
    std::string string_key_next_peer = "";

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        //Common::Print_or_log pl;
        //pl.handle_print_or_log({it->key().ToString(), ":", it->value().ToString()});
        if (it->key().ToString() > key)
        {
            string_key_next_peer = it->key().ToString();
            break;
        }
    }

    if (string_key_next_peer == "")
    {
        // if next peer is the first in whole db, go search from start
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            string_key_next_peer = it->key().ToString();
            break;
        }
    }

    delete it;

    return string_key_next_peer;
}

std::string Rocksy::FindServerPeer(std::string &key)
{
    std::string string_key_server_peer;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        if (it->key().ToString() >= key)
        {
            Common::Print_or_log pl;
            pl.handle_print_or_log({"it:", it->value().ToString()});
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

std::string Rocksy::FindNextServerPeer(std::string &string_key)
{
    std::istringstream iss (string_key);
    uint256_t key;
    iss >> key;
    if (iss.fail())
    {
        Common::Print_or_log pl;
    pl.handle_print_or_log({"ERROR in creating the uint for the key!"});
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

uint256_t Rocksy::TotalAmountOfPeers() // TODO: shouldn't uint256_t should be used?
{
    std::string string_num;
    db->GetProperty("rocksdb.estimate-num-keys", &string_num);

    std::istringstream iss (string_num);
    uint256_t num;
    iss >> num;
    if (iss.fail())
    {
        Common::Print_or_log pl;
        pl.handle_print_or_log({"ERROR in creating the uint for the total_amount_of_peers!"});
        return 1;
    }

    return num;
}

uint256_t Rocksy::CountPeersFromTo(std::string &from, std::string &to)
{
    uint256_t count = 0;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    if (from < to)
    {
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            if (it->key().ToString() > from)
            {
                // Common::Print_or_log pl;
                // pl.handle_print_or_log({"it->key():", it->key().ToString()});
                count++;

                if (it->key().ToString() >= to)
                {
                    break;
                }
            }
        }
    }
    else if (from >= to)
    {
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            if (it->key().ToString() > from)
            {
                // Common::Print_or_log pl;
                // pl.handle_print_or_log({"it->key():", it->key().ToString()});
                count++;
            }
        }

        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            count++;

            if (it->key().ToString() >= to)
            {
                break;
            }
        }
    }

    delete it;

    return count;
}

std::string Rocksy::FindPeerFromTillCount(std::string &key, uint256_t &count)
{
    std::string string_key_counted_peer;
    uint256_t counter = 0;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        if (count == 0)
        {
            string_key_counted_peer = "0";
            break;
        }

        // Common::Print_or_log pl;
        // pl.handle_print_or_log({it->key().ToString(), ":", it->value().ToString()});
        if (it->key().ToString() > key)
        {
            counter++;

            if (counter == count)
            {
                string_key_counted_peer = it->key().ToString();
                break;
            }
        }
    }

    // if next peer is the first in whole level db, go search from start
    if (counter < count)
    {
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            counter++;

            if (counter == count)
            {
                string_key_counted_peer = it->key().ToString();
                break;
            }
        }
    }

    delete it;

    return string_key_counted_peer;
}

void Rocksy::DatabaseDump()
{
    Common::Print_or_log pl;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        pl.handle_print_or_log({"DatabaseDump:", it->key().ToString()});
    }
}

std::vector<std::string> Rocksy::GetPeersInRange(uint256_t from, uint256_t till)
{
    std::vector<std::string> peers_in_range;
    uint256_t count = 0;

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        count++;

        if (count >= from && count <= till)
        {
            peers_in_range.push_back(it->key().ToString());
        }
        else if (count > till) break;
    }
    delete it;

    return peers_in_range;
}