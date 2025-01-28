#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT "3490"

int main(int argc, const char * argv[]) 
{
    int sockfd, new_fd;
    
    int status;
    struct addrinfo hints, *servinfo;
    
    struct sockaddr_storage client_addr;
    
    char ipstr[INET6_ADDRSTRLEN]; //Array size for IP6, to cast and print IP.
    
    bzero(&hints, sizeof(hints)); //Empty the structure.
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; //Use host IP.
    
    if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    struct sockaddr_in *ipv4 = (struct sockaddr_in *)servinfo->ai_addr; //Cast the generic structure to an IPV4 structure.
    inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, INET_ADDRSTRLEN); //Translate addr from bin to string.
    printf("IP address: %s\n", ipstr);
    
    
    freeaddrinfo(servinfo); //Free the linked list from memory.
    
    return 0;
}
