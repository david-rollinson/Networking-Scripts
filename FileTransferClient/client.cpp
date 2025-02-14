#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fstream>

#define PORT "3490" // The port the client will be connecting to
#define MAXDATASIZE 100

void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr); // Return the IPv4 address format
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr); // Return the IPv6 address format
}

int main(int argc, char *argv[]){
    int sockfd = 0;
    int numbytes;
    int opt = 1;
    char buf[MAXDATASIZE];
    
    // Setup data to send
    char const *filename = "example.txt"; // Store file name as string literal (for C compatibility)
    std::ofstream file("example.txt", std::ios::binary); // Define a file to open in binary mode
    if (!file.is_open()) { // Check if file is open elsewhere
        perror("");
        return 1;
    }
    
    struct addrinfo hints, *servinfo, *p; // Create the structures for connection settings, getaddrinfo and bindings
    int status;
    char cipstr[INET6_ADDRSTRLEN]; // Client array to cast and print IP
    
    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n"); // Check CLI usage
        exit(1);
    }
    
    bzero(&hints, sizeof(hints)); // Set the structure to empty
    hints.ai_family = AF_UNSPEC; // Agnostic IP family
    hints.ai_socktype = SOCK_STREAM;
    
    if ((status = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) { // Pass in address from CMD, get information and store it inside the servinfo structure
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }
    
    // Connect to the first socket we can
    for(p = servinfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }
        
        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("client: connect");
            continue;
        }
        
        break;
    }
    
    // Set socket option to reuse port or address
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
        perror("setsockopt");
        exit(1);
    }
    
    if(p == NULL){
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), cipstr, INET6_ADDRSTRLEN);
    printf("client: connectign to &s\n", cipstr);
    
    freeaddrinfo(servinfo);
    
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sockfd);
}
