#include "p2p_handler.hpp"
  
//using namespace std; 
using namespace crowd;

std::string make_daytime_string()
{
  using namespace std; // For time_t, time and ctime;
  time_t now = time(0);
  return ctime(&now);
}

int p2p_handler::server_main()
{
    try
    {
        boost::asio::io_service io_service;

        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 1975));

        for (;;)
        {
        tcp::socket socket(io_service);
        acceptor.accept(socket);

        std::string message = make_daytime_string();

        boost::system::error_code ignored_error;
        boost::asio::write(socket, boost::asio::buffer(message),
            boost::asio::transfer_all(), ignored_error);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}