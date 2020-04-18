#include <gtest/gtest.h>
#include "gmock/gmock.h"

#include "poco.hpp"

using ::testing::Return;
using ::testing::_;

using namespace Crowd;

class MockPoco : public Poco
{
    public:
        MockPoco(): Poco() {}
        MOCK_METHOD1(Get, std::string (uint32_t key));
        MOCK_METHOD2(Put, int (uint32_t key, std::string value));
        MOCK_METHOD1(Delete, int (uint32_t key));
        MOCK_METHOD1(FindChosenOne, uint32_t (uint32_t key));
};

class MockPocoTest : public ::testing::Test
{
    protected:
        void SetUp() override
        {
            mp.Put(2, "test2");
            mp.Put(4, "test4");
            mp.Put(5, "test5");
        }

        // void TearDown() override {}

        MockPoco mp;
};

TEST_F(MockPocoTest, GetAndPut)
{
    EXPECT_CALL(mp, Get(1)).WillOnce(Return(""));
    EXPECT_CALL(mp, Get(2)).WillOnce(Return("test2"));
    mp.Get(1);
    mp.Get(2);
}

TEST_F(MockPocoTest, Delete)
{
    EXPECT_CALL(mp, Delete(5)).WillOnce(Return(0));
    mp.Delete(5);
}

TEST_F(MockPocoTest, FindChosenOne)
{
    EXPECT_CALL(mp, FindChosenOne(3)).WillOnce(Return(4));
    EXPECT_CALL(mp, FindChosenOne(4)).WillOnce(Return(4));
    mp.FindChosenOne(3);
    mp.FindChosenOne(4);
}
