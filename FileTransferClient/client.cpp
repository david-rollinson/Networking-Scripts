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

void* format_in_addr(struct sockaddr *sa){ // Returns a generic pointer to handle multiple return types
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr); // Cast the structure according to the IP version
    } else { // Implies AF_INET6
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
}

int main(int argc, char *argv[]){
    int sockfd = 0;
    int opt = 1; // Set socket reuse option
    char buf[MAXDATASIZE];
    
    // Setup data to send
    int numbytes;
    char const *filename = "example.txt"; // Store file name as string literal (for C compatibility)
    std::ofstream file("example.txt", std::ios::binary); // Define a file to open in binary mode
    if (!file.is_open()) { // Check if file is open elsewhere
        printf("Could not open file");
        return 1;
    }
    
    struct addrinfo hints, // Create structure to hold socket options
        *servinfo, *p; // For dynamic allocation on the heap due to size unknown (getaddrinfo result could be a linked list)
    int status;
    char cipstr[INET6_ADDRSTRLEN]; // Client array to cast and print IP
    
    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n"); // Check CLI usage
        exit(1);
    }
    
    memset(&hints, 0, sizeof(hints)); // Fill the structure with 0
    hints.ai_family = AF_UNSPEC; // Agnostic IP family
    hints.ai_socktype = SOCK_STREAM;
    // We don't need to set ai_flags because we're not using a passive connection, nor require other options e.g. AI_NUMERICHOST
    
    if ((status = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) { // Initialise servinfo reference value
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2; // Return under the custom error condition
    }
    
    // Connect to the first result available
    for(p = servinfo; p != NULL; p = p->ai_next){ // Copy to iterator so we can maintain access and free servinfo later
        
        // Create the socket
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){ // Syscall returns -1 on err
            perror("client: socket");
            continue;
        }
        
        // Set socket option to reuse port or address
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
            perror("setsockopt");
            exit(1);
        }
        
        // Connect on the socket
        if(connect(sockfd, p->ai_addr, p->ai_addrlen) < 0){
            close(sockfd); // Close the file descriptor for reuse
            perror("client: connect");
            continue;
        }
        
        break;
    }
    
    if(p == NULL){
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
     
    // Translate result to printable format
    if(inet_ntop(p->ai_family, format_in_addr(p->ai_addr), cipstr, p->ai_addrlen) != NULL){
        printf("client: connecting to %s\n", cipstr);
    } else {
        printf("client: IP failed to translate from network to host bit-order\n");
    }
    
    freeaddrinfo(servinfo);
    
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) < 0) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sockfd);
    
    return 0;
}
