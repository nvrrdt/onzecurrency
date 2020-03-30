#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mock_p2p_handler.hpp"

using ::testing::Return;
using ::testing::_;

using namespace crowd;

class MockP2pHandler : public p2p_handler
{
    public:        
        MOCK_METHOD2(p2p_switch, void (std::string task_client, std::string ip_chosen_one));
        MOCK_METHOD0(parse_ip_peers_json,vector<string> ());
        MOCK_METHOD2(client, std::string (std::string& ip_adress, std::string& task_client));
        MOCK_METHOD0(server_main, int ());
        MOCK_METHOD1(getDataServer, string (tcp::socket&));
        MOCK_METHOD2(sendDataServer, void (tcp::socket&, const string&));
        MOCK_METHOD1(getDataClient, string (tcp::socket&));
        MOCK_METHOD2(sendDataClient, void (tcp::socket&, const string&));
        MOCK_METHOD1(save_blockchain, void (string));
        
};

TEST(P2pTest, TaskEqDownload)
{
    MockP2pHandler p2p;

    std::string ip = "localhost";
    std::string tc = "download";

    EXPECT_CALL(p2p, client(_, _))
        //.WillOnce(Return("download"));
        .WillOnce(DoAll(testing::SetArgReferee<0>(ip), testing::SetArgReferee<1>(tc)))
        .WillOnce(Return("download"));
}



int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}