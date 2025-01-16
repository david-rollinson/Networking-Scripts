#include <iostream>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "main.h"

// AF_INET - Address family | PF_INET - Protocol family

int main(int argc, char *argv[])
{
    printf("Hello, World!\n");
    
    struct sockaddr_in sa;
    char ip4[INET_ADDRSTRLEN];
    
    inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr));
    inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
    
    printf("The IPv4 address is: %s\n", ip4);
    printf("The decimal represenation is: %u\n", ntohl(sa.sin_addr.s_addr)); //This requires the endinanness of the binary IP to be adjusted to suit the host printable form. (Apple ARM chips and Intel chips both use little-endian systems.)
    
    struct sockaddr_in6 sa6;
    struct in6_addr ia6 = IN6ADDR_ANY_INIT;
    char ip6[INET6_ADDRSTRLEN];
    
    sa6.sin6_addr = ia6;
    inet_pton(AF_INET6, "2001:db8:8c00:22::171", &(sa6.sin6_addr));
    inet_ntop(AF_INET6, &(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);
    printf("The IPv6 address is: %s\n", ip6);
    
    int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
                    const char *service,  // e.g. "http" or port number
                    const struct addrinfo *hints,
                    struct addrinfo **res);
    
    return 0;
}

int connect_to_server() {
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo; //this will point to the results.
    
    memset(&hints, 0, sizeof hints); //make sure the struct is empty.
    hints.ai_family = AF_UNSPEC; //don't care IPv4 or IPv6.
    hints.ai_socktype = SOCK_STREAM; //TCP stream sockets.
    hints.ai_flags = AI_PASSIVE; //fill in my IP for me.
    
    if((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
    
    // servinfo now points to a linked list of 1 or more struct addrinfos.

    // ... do everything until you don't need servinfo anymore ....

    freeaddrinfo(servinfo); // free the linked-list.
    
    return 0;
}

int test_network(int argc, char *argv[2]) {
    
    struct addrinfo hints, *res, *p;
    int status;
    
    char ipstr[INET6_ADDRSTRLEN];
    
    if (argc != 2) {
        fprintf(stderr, "Usage: showip hostname\n");
        return 1;
    }
    
    memset(&hints, 0, sizeof(hints)); //Make sure the addr storage structure is empty.
    /*The hints argument points to an addrinfo structure that specifies
     criteria for selecting the socket address structures returned in
     the list pointed to by res.*/
    hints.ai_family = AF_UNSPEC; //Change to AF_INET or AF_INET6 to force version.
    hints.ai_socktype = SOCK_STREAM; //The type of connection we want to establish.
    
    if((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0){ //Here the result gets passed into 'res', based upon the spec of hints. The NULL here is because we don't want to connect to a specific port at the host/'node' address in this instance.
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }
    
    
    return 0;
}

