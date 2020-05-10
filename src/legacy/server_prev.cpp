#include <iostream>
#include <fstream>
#include <string>

#include "p2p.hpp"
  
//using namespace std; 
using namespace Crowd;

int P2p::server_main()
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

            std::ifstream ifile("./blockchain/block0000000000.json", ios::in);

            if (ifile.is_open())
            {
                std::string line;
                while ( std::getline (ifile,line) )
                {
                    cout << "line: " << line << '\n';
                    P2p::sendDataServer(socket_server, line);
                }
                ifile.close();
            }
            else cout << "Unable to open file";
        }
        else if (task == "verify") // verification of the block by the chosen_one
        {
            std::cout << "verification initiated!!" << std::endl;
        }

        while (true)
        {
            // Fetching response 
            response = P2p::getDataServer(socket_server); 
    
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
string P2p::getDataServer(tcp::socket& socket) 
{ 
    boost::asio::streambuf buf; 
    boost::asio::read_until(socket, buf, "\n"); 
    string data = buffer_cast<const char*>(buf.data()); 
    return data; 
}

// Driver program to send data 
void P2p::sendDataServer(tcp::socket& socket, const string& message) 
{ 
    boost::asio::write(socket, boost::asio::buffer(message + "\n")); 
}
