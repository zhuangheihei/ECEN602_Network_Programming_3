#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
//Network header file
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[]){
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // TCP stream sockets
    if ((status = getaddrinfo(argv[1], argv[2] , &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
    
    int servsock;
    if(servsock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol) == -1){
        perror("linstener: socket");
        exit(1);
    }
    
    if (bind(servsock, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
            close(servsock);
            perror("listener: bind");
            exit(1);
    }
    
}