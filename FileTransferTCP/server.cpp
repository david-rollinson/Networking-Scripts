#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"
#define BACKLOG 10

void* format_in_addr(struct sockaddr *sa){ // Returns a generic pointer
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr); // Cast the structure according to the IP version
    } else { // Implies AF_INET6
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
}

int main(int argc, const char * argv[]) {
    struct addrinfo hints, *servinfo, *p;

    struct sockaddr_storage client_addr;
    socklen_t sin_size; // Store client address size (opaque 32bit int)
    
    int sockfd, new_fd; // Listen fd, new conn fd
    int status;
    int opt = 1; // Used later to reuse socket
    char sipstr[INET_ADDRSTRLEN]; // Server array to cast and print IP
    char cipstr[INET6_ADDRSTRLEN]; // Client array to cast and print IP
    
    bzero(&hints, sizeof(hints)); // Empty the structure
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // Use host IP
    
    if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) { // Get address information and store it inside the servinfo structure
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }
    
    for(p = servinfo; p != NULL; p = p->ai_next) {
        // Print the current IP
        inet_ntop(AF_INET, format_in_addr(p->ai_addr), sipstr, INET6_ADDRSTRLEN); // Translate addr from bin to string
        printf("Current IP: %s\n", sipstr);
        
        // Create a socket
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket");
            continue;
        }
        
        // Set socket option to reuse port or address
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
            perror("setsockopt");
            exit(1);
        }
        
        // Bind the port to the socket
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd); // Close the socket if not bound
            perror("server: bind");
            continue;
        }
        
        break;
    }
    
    freeaddrinfo(servinfo); // Free the linked list from memory
    
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    
    if(listen(sockfd, BACKLOG) == -1){
        perror("listen");
        exit(1);
    }
    
    printf("server: waiting for connections...\n");
    
    while(1){ // Loop will continue infinitely until broken - main accept() loop
        sin_size = sizeof(client_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
        if(new_fd == -1){
            perror("accept");
            continue;
        }
        
        // Print the current IP
        inet_ntop(client_addr.ss_family, format_in_addr((struct sockaddr *)&client_addr), cipstr, INET6_ADDRSTRLEN); // Translate addr from bin to string
        printf("Accepted connection to IP: %s\n", cipstr);
        
    }
    
    return 0;
}
