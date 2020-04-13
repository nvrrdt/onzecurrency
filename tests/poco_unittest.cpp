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

TEST(PocoTest, Get)
{
    MockPoco mp;
    
    EXPECT_CALL(mp, Get(1)).Times(1);
    ON_CALL(mp, Get(1))
        .WillByDefault(Return(""));

    mp.Get(1);    
}

TEST(PocoTest, PutAndGet)
{
    MockPoco mp;
    
    EXPECT_CALL(mp, Put(1, "test")).Times(1);
    EXPECT_CALL(mp, Get(1)).Times(1);
    ON_CALL(mp, Put(1, "test"))
        .WillByDefault(Return(0));
    ON_CALL(mp, Get(1))
        .WillByDefault(Return("test"));

    mp.Put(1, "test");
    mp.Get(1);    
}

TEST(PocoTest, PutAndDelete)
{
    MockPoco mp;

    EXPECT_CALL(mp, Put(1, "test")).Times(1);
    EXPECT_CALL(mp, Delete(1)).Times(1);
    ON_CALL(mp, Put(1, "test"))
        .WillByDefault(Return(0));
    ON_CALL(mp, Delete(1))
        .WillByDefault(Return(0));

    mp.Put(1, "test");
    mp.Delete(1); 
}

TEST(PocoTest, FindChosenOne1)
{
    MockPoco mp;

    EXPECT_CALL(mp, FindChosenOne(2)).Times(1);
    ON_CALL(mp, FindChosenOne(2))
        .WillByDefault(Return(3));

    mp.Put(1, "test1");
    mp.Put(3, "test3");
    mp.Put(4, "test4");
    mp.FindChosenOne(2);
}

TEST(PocoTest, FindChosenOne2)
{
    MockPoco mp;

    EXPECT_CALL(mp, FindChosenOne(2)).Times(1);
    ON_CALL(mp, FindChosenOne(2))
        .WillByDefault(Return(2));

    mp.Put(1, "test1");
    mp.Put(2, "test2");
    mp.Put(3, "test3");
    mp.Put(4, "test4");
    mp.FindChosenOne(2);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}