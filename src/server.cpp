#include "p2p_handler.hpp"
  
//using namespace std; 
using namespace crowd;

// Driver program for receiving data from buffer 
string p2p_handler::getData(tcp::socket& socket) 
{ 
    boost::asio::streambuf buf; 
    boost::asio::read_until(socket, buf, "\n"); 
    string data = buffer_cast<const char*>(buf.data()); 
    return data; 
} 
  
// Driver program to send data 
void p2p_handler::sendData(tcp::socket& socket, const string& message) 
{ 
    write(socket, 
          buffer(message + "\n")); 
}