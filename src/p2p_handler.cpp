// this one manages p2p behaviour
// always listening server
// unless polling the network as a client

#include "p2p_handler.hpp"
  
using namespace crowd;

void p2p_handler::p2p_switch()
{
    io_service io_service; 
  
    // Listening for any new incomming connection 
    // at port 9999 with IPv4 protocol 
    tcp::acceptor acceptor_server( 
        io_service, 
        tcp::endpoint(tcp::v4(), 9999)); 
  
    // Creating socket object 
    tcp::socket server_socket(io_service); 
  
    // waiting for connection 
    acceptor_server.accept(server_socket); 
  
    // Reading username 
    string u_name = getData(server_socket); 
    // Removing "\n" from the username 
    u_name.pop_back(); 
  
    // Replying with default mesage to initiate chat 
    string response, reply; 
    reply = "Hello " + u_name + "!"; 
    cout << "Server: " << reply << endl; 
    sendData(server_socket, reply); 
  
    while (true) { 
  
        // Fetching response 
        response = getData(server_socket); 
  
        // Popping last character "\n" 
        response.pop_back(); 
  
        // Validating if the connection has to be closed 
        if (response == "exit") { 
            cout << u_name << " left!" << endl; 
            break; 
        } 
        cout << u_name << ": " << response << endl; 
  
        // Reading new message from input stream 
        cout << "Server"
             << ": "; 
        getline(cin, reply); 
        sendData(server_socket, reply); 
  
        if (reply == "exit") 
            break; 
    } 
}