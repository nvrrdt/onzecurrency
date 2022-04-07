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

std::pair<std::string, uint256_t> DatabaseSharding::get_fair_order_nr(std::string user_id)
{
    Common::Print_or_log pl;

    Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");
    std::string zero = "0";
    uint256_t order_nr = rocksy->CountPeersFromTo(zero, user_id);
    delete rocksy;

    std::map<std::string, uint256_t> fair_shards = fair_partitioning();

    std::pair<std::string, uint256_t> fair_order_nr;
    uint256_t total_users = fair_shards["total_users"];
    uint256_t users_distance = fair_shards["users_distance"];
    uint256_t users_remainder = fair_shards["users_remainder"];
    if (order_nr <= total_users)
    {
        fair_order_nr.first = "ok";

        if (order_nr <= users_remainder)
        {
            // remainder has one more user in distance
            fair_order_nr.second = order_nr * (users_distance + 1);
        }
        else
        {
            fair_order_nr.second = ((order_nr - users_remainder) * users_distance) + (users_remainder * (users_distance + 1));
        }
    }
    else
    {
        fair_order_nr.first = "error";
        fair_order_nr.second = 0;

        pl.handle_print_or_log({"ERROR: total_users < order_nr"});
    }

    return fair_order_nr;
}

// calculation of dynamic sharding
uint32_t DatabaseSharding::get_amount_of_shards()
{
    Crowd::Rocksy* rocksy = new Crowd::Rocksy("usersdbreadonly");
    uint256_t total_users = rocksy->TotalAmountOfPeers();
    delete rocksy;

    int x = 0; // amount of shards: 2^x
    for (;;)
    {
        // limit at 128 users per shard until maximum of 128 shards (2^7)
        if (total_users <= static_cast<uint256_t>(128 * pow(2, x)) && x < 7)
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