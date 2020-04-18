#include <gtest/gtest.h>
#include "gmock/gmock.h"

#include "configdir.hpp"

#define GTEST_COUT std::cerr << "[ INFO ] "

using ::testing::Return;
using ::testing::_;
using ::testing::EndsWith;

using namespace Crowd;

TEST(ConfigDirTest, GetConfigDir)
{
    ConfigDir cd;
    EXPECT_THAT(cd.GetConfigDir(), EndsWith(".config/onzehub/"));
}

