#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

//Network header file
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define buffer_size 512

using namespace std;

int generate_ephemeral_port() //generates random port between 1024 and 65535
{
	unsigned short int value;
	value = (rand()%64512) + 1024;	
}

int main(int argc, char *argv[]){
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((status = getaddrinfo(NULL, argv[1] , &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(0);
    }
    
    int servsock;
    if((servsock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1){
        perror("linstener: socket");
        exit(0);
    }
    cout << "Socket established." << endl;
    
    if ((bind(servsock, servinfo->ai_addr, servinfo->ai_addrlen)) == -1) {
        close(servsock);
        perror("listener: bind");
        exit(0);
    }
    cout << "Socket binded." << endl;
    
    cout << "Waiting to recvfrom ..." << endl;
    
    while(true){
        //call recvfrom to get a RRQ from client
        
        
        
    }
    
    
    exit(1);
}