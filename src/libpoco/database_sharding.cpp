#include "sharding.hpp"

#include "rocksy.hpp"

using namespace Poco;

/**
 * Sharding done as explained in 'papers/tackling_the_scaling_problem.md'
 * This is proof-of-chosen-ones v2.
 * 
 * @brief Dynamically partitioning the total_users in rocksdb into 1 to 128 shards
 */

void DatabaseSharding::partitioning()
{
    //
}