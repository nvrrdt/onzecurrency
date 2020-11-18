#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <netinet/in.h>

#include <string>
 
#include "p2p.hpp"

using namespace Crowd;

// https://wuyongzheng.wordpress.com/2013/01/31/experiment-on-tcp-hole-punching/
int Udp::tcp_peer (std::string s_remotehost)
{
    char* c_remotehost;
    c_remotehost = &s_remotehost[0];

    int sock, port = 975;
    struct sockaddr_in addr;
    char buff[256];
 
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        die("socket() failed");
 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(sock, (const struct sockaddr *)&addr, sizeof(addr)))
        Udp::die("bind() failed\n");
 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(c_remotehost);
    addr.sin_port = htons(port);
 
    while (connect(sock, (const struct sockaddr *)&addr, sizeof(addr))) {
        if (errno != ETIMEDOUT) {
            perror("connect() failed. retry in 2 sec.");
            sleep(2);
        } else {
            perror("connect() failed.");
        }
    }
 
    snprintf(buff, sizeof(buff), "Hi, I'm %d.", getpid());
    printf("sending \"%s\"\n", buff);
    if (send(sock, buff, strlen(buff) + 1, 0) != strlen(buff) + 1)
        die("send() failed.");
 
    if (recv(sock, buff, sizeof(buff), 0) <= 0)
        die("recv() failed.");
    printf("received \"%s\"\n", buff);
 
    return 0;
}