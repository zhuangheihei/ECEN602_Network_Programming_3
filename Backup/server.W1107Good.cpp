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
    
    if(argc != 3){ //if input argument is not exactly 3, return an error
        cerr << "usage: server ip port" << endl;  //cerr is cpp's error handler
        exit(0);
    }
    
    //declare variables and data structures
    int status; 
    int servsock; //server socket
    
    fd_set master;// Create a master set of file discriptor.
    fd_set read_fds;  // temp file descriptor list for select()
    FD_ZERO (&master);// Clear all entries in set.
    FD_ZERO (&read_fds);// Clear all entries in set.
    int maxfd;
    
    struct addrinfo hints;
    struct addrinfo *servinfo;
    struct sockaddr_storage clientinfo, tempinfo;
    
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // TCP stream sockets
    
    if ((status = getaddrinfo(argv[1], argv[2] , &hints, &servinfo)) != 0) {
        cerr<< "getaddrinfo error: %s\n"<< gai_strerror(status) << endl;
        exit(0);
    }
    
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
    
    // while(true){
    //     //call recvfrom to get a RRQ from client
        
        
        
    // }
    
    
    exit(1);
}