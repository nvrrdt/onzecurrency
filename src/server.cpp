#include "p2p_handler.hpp"
  
//using namespace std; 
using namespace crowd;

int p2p_handler::server_main()
{
    try
    {
        boost::asio::io_service io_service;

        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 1975));

        tcp::socket socket_server(io_service);
        acceptor.accept(socket_server);

        // Reading task 
        string task = getDataServer(socket_server); 
        // Removing "\n" from the username 
        task.pop_back(); 
    
        // Replying with default mesage to initiate chat 
        string response;
        if (task == "download")
        {
            std::cout << "download initiated!!" << std::endl;
            p2p_handler::sendDataServer(socket_server, task); // TODO: change task for a download from the blockchain
        } 

        while (true)
        {
            // Fetching response 
            response = p2p_handler::getDataServer(socket_server); 
    
            // Popping last character "\n" 
            response.pop_back();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

// Driver program for receiving data from buffer 
string p2p_handler::getDataServer(tcp::socket& socket) 
{ 
    boost::asio::streambuf buf; 
    boost::asio::read_until(socket, buf, "\n"); 
    string data = buffer_cast<const char*>(buf.data()); 
    return data; 
}

// Driver program to send data 
void p2p_handler::sendDataServer(tcp::socket& socket, const string& message) 
{ 
    boost::asio::write(socket, boost::asio::buffer(message + "\n")); 
}