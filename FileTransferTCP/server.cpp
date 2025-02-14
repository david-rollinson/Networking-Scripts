
// Much of this code is from: https://beej.us/guide/bgnet/html/#a-simple-stream-server

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

void handle_sigchild(int s){
    
    int save_errno = errno;
    
    while(waitpid(-1, NULL, WNOHANG) > 0); // Exits if a child PID isn't returned
    
    errno = save_errno;
    
}

void* format_in_addr(struct sockaddr *sa){ // Returns a generic pointer to handle multiple return types
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr); // Cast the structure according to the IP version
    } else { // Implies AF_INET6
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
}

int get_addrlen(sa_family_t family){ // Helper function to determine address length
    switch (family) {
        case AF_INET:
            return INET_ADDRSTRLEN;
        case AF_INET6:
            return INET6_ADDRSTRLEN;
        default:
            fprintf(stderr, "Unknown address family: %d\n", family);
            return -1;
    }
}

int main(int argc, const char * argv[]) {
    struct addrinfo hints, 
        *servinfo, *p; // For dynamic allocation on the heap due to size unknown (getaddrinfo result could be a linked list)

    struct sockaddr_storage client_addr; // Size-agnostic struct for future casting
    socklen_t sin_size; // Store client address size in opaque 32bit int
    
    struct sigaction sa; // Setup signal handler for child process cleanup
    
    int sockfd, new_fd; // Listen fd, new conn fd
    int status;
    int opt = 1; // Set socket reuse option
    char sipstr[INET6_ADDRSTRLEN]; // Server array to cast and print IP
    char cipstr[INET6_ADDRSTRLEN]; // Client array to cast and print IP
    
    // SETUP SOCKET OPTIONS
    bzero(&hints, sizeof(hints)); // Empty the structure
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // Use host IP
    
    if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) { // Initialise servinfo reference value
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }
    
    // Bind to the first result available
    for(p = servinfo; p != NULL; p = p->ai_next) { // Copy to iterator so we can maintain access and free servinfo later
        // Translate result to printable format
        if(inet_ntop(p->ai_family, format_in_addr(p->ai_addr), sipstr, p->ai_addrlen) != NULL){
            printf("Current IP: %s\n", sipstr);
        } else {
            printf("IP failed to translate from network to host bit-order\n");
        }
        
        // Create the socket
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){ // Syscall returns -1 on err
            perror("server: socket");
            continue;
        }
        
        // Set socket option to reuse port or address
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
            perror("setsockopt");
            exit(1);
        }
        
        // Bind the port to the socket
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) < 0){
            close(sockfd); // Close the socket if not bound
            perror("server: bind");
            continue;
        }
        
        break;
    }
    
    freeaddrinfo(servinfo); // Free the linked list from memory
    
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n"); // Print to the error stream
        exit(1);
    }
    
    if(listen(sockfd, BACKLOG) < 0){
        perror("listen");
        exit(1);
    }
    
    sa.sa_handler = handle_sigchild; // Reap dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("sigaction");
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
        
        // Translate result to printable format - helper function gets size according to address family
        if(inet_ntop(client_addr.ss_family, format_in_addr((struct sockaddr *)&client_addr), cipstr, get_addrlen(client_addr.ss_family)) != NULL){
            printf("server: accepted connection to IP: %s\n", cipstr);
        } else {
            printf("server: IP failed to translate from network to host bit-order\n");
        }
        
    }
    
    return 0;
}
