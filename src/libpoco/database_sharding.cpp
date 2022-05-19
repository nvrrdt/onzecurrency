#include "sharding.hpp"

#include "rocksy.hpp"

#include <math.h>
#include <limits>

using namespace Poco;

/**
 * Sharding done as explained in 'papers/tackling_the_scaling_problem.md'
 * This is proof-of-chosen-ones v2.
 * 
 * @brief Dynamically partitioning the total_users in rocksdb into 1 to 128 shards
 */

std::map<std::string, uint256_t> DatabaseSharding::fair_partitioning()
{
    Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");
    uint256_t total_users = rocksy->TotalAmountOfPeers();
    delete rocksy;
    uint256_t uint256_max = std::numeric_limits<uint256_t>::max();
    uint256_t users_distance = uint256_max / total_users;
    uint256_t users_remainder = uint256_max % total_users;

    std::map<std::string, uint256_t> fair_shards;
    fair_shards["total_users"] = total_users;
    fair_shards["users_distance"] = users_distance;
    fair_shards["users_remainder"] = users_remainder;

    return fair_shards;
}

std::pair<std::string, uint256_t> DatabaseSharding::get_fair_user_id(std::string user_id)
{
    Common::Print_or_log pl;

    Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");
    std::string zero = "0";
    uint256_t order_nr = rocksy->CountPeersFromTo(zero, user_id);
    delete rocksy;

    std::map<std::string, uint256_t> fair_shards = fair_partitioning();

    std::pair<std::string, uint256_t> fair_user_id;
    uint256_t total_users = fair_shards["total_users"];
    uint256_t users_distance = fair_shards["users_distance"];
    uint256_t users_remainder = fair_shards["users_remainder"];
    if (order_nr <= total_users)
    {
        fair_user_id.first = "ok";

        if (order_nr <= users_remainder)
        {
            // remainder has one more user in distance
            fair_user_id.second = order_nr * (users_distance + 1);
        }
        else
        {
            fair_user_id.second = ((order_nr - users_remainder) * users_distance) + (users_remainder * (users_distance + 1));
        }
    }
    else
    {
        fair_user_id.first = "error";
        fair_user_id.second = 0;

        pl.handle_print_or_log({"ERROR: total_users < order_nr"});
    }

    return fair_user_id;
}

// calculation of dynamic sharding
uint32_t DatabaseSharding::get_amount_of_shards()
{
    Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");
    uint256_t total_users = rocksy->TotalAmountOfPeers();
    delete rocksy;

    int x = 0; // amount of shards: 2^x
    Common::Globals globals;
    for (;;)
    {
        // limit at 128 users per shard until maximum of 128 shards (2^7) --> 7 == preliminary max_pow (please verify)
        if (total_users >= static_cast<uint256_t>(128 * pow(2, x)) && x < globals.get_max_pow())
        {
            x++;
        }
        else
        {
            break;
        }
    }

    uint32_t shard_amount = pow(2, x);
    return shard_amount;
}

std::pair<uint256_t, uint256_t> DatabaseSharding::get_fair_shard_range(std::string user_id)
{
    auto fair_user_id = get_fair_user_id(user_id); // TODO it isn't used correctly, did I mean to do something else?
    std::pair<uint256_t, uint256_t> range;
    if (fair_user_id.first == "ok")
    {
        auto amount_of_shards = get_amount_of_shards();

        for (int i = 0; i < amount_of_shards; i++)
        {
            // (0 3)(4 7)(8 11)(12 15)(16 19)
            range.first = i * ((std::numeric_limits<uint256_t>::max() / amount_of_shards) + 1);
            range.second = range.first + (std::numeric_limits<uint256_t>::max() / amount_of_shards);
        }
    }
    else
    {
        range.first = 0, range.second = 0;
    }

    return range;
}

std::vector<std::string> DatabaseSharding::get_shard_users(std::string user_id)
{
    Common::Print_or_log pl;

    auto fair_user_id = get_fair_user_id(user_id);

    std::vector<std::string> shard_users = {};
    std::pair<uint256_t, uint256_t> range;
    uint32_t shard_nr;
    if (fair_user_id.first == "ok")
    {
        auto amount_of_shards = get_amount_of_shards();

        // calculate the shard_nr for the fair_user
        for (int i = 0; i < amount_of_shards; i++)
        {

            // (0 3)(4 7)(8 11)(12 15)(16 19)
            range.first = i * ((std::numeric_limits<uint256_t>::max() / amount_of_shards) + 1);
            range.second = range.first + (std::numeric_limits<uint256_t>::max() / amount_of_shards);

            if (fair_user_id.second >= range.first && fair_user_id.second < range.second)
            {
                shard_nr = i;
                break;
            }
        }

        Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");
        uint256_t total_users = rocksy->TotalAmountOfPeers();

        // calculate the range for that shard
        for (int i = 0; i < amount_of_shards; i++)
        {
            range.first = i * (std::numeric_limits<uint256_t>::max() / amount_of_shards);
            range.second = range.first + (std::numeric_limits<uint256_t>::max() / amount_of_shards) - 1;

            if (shard_nr == i) break;
        }

        // lookup the user with the order_nr in rocksy
        shard_users = rocksy->GetPeersInRange(range.first, range.second);

        delete rocksy;
    }
    else
    {
        shard_users.push_back("error");
    }

    return shard_users;
}