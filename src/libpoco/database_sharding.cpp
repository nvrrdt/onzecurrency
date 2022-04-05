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
    uint256_t shard_distance = uint256_max / total_users;
    uint256_t shard_remainder = uint256_max % total_users;

    std::map<std::string, uint256_t> shards;
    shards["total_users"] = total_users;
    shards["shard_remainder"] = shard_remainder;
    shards["shard_distance"] = shard_distance;

    return shards;
}