#include <iostream>
#include <string>

#include "p2p.hpp"

using namespace Crowd;

std::string P2p::client(string& ip_adress, string& task_client) 
{ 
    string port = "1975";

    try
    {
        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(ip_adress, port);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;

        tcp::socket socket_client(io_service);
        boost::system::error_code error = boost::asio::error::host_not_found;
        while (error && endpoint_iterator != end)
        {
            socket_client.close();
            socket_client.connect(*endpoint_iterator++, error);
        }
        if (error)
        throw boost::system::system_error(error);

        // Sending username to another end 
        // to initiate the conversation
        P2p::sendDataClient(socket_client, task_client);

        string response;

        while (true)
        {
            // Fetching response 
            response = P2p::getDataClient(socket_client); 
    
            // Popping last character "\n" 
            response.pop_back();

            std::cout << "Response client is: " << response << std::endl;

            // Validating if the connection has to be closed 
            /*if (response == "download") { 
                cout << "Response: Connection terminated"  << endl; 
                break; 
            } */
    
            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            return response;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return to_string(0);
}

string P2p::getDataClient(tcp::socket& socket) 
{ 
    boost::asio::streambuf buf; 
    boost::asio::read_until(socket, buf, "\n"); 
    string data = buffer_cast<const char*>(buf.data()); 
    return data; 
} 
  
void P2p::sendDataClient(tcp::socket& socket, const string& message) 
{ 
    boost::asio::write(socket, boost::asio::buffer(message + "\n")); 
} 