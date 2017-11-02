#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

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
    
    struct sigaction sa;
    
    fd_set master;// Create a master set of file discriptor.
    fd_set read_fds;  // temp file descriptor list for select()
    FD_ZERO (&master);// Clear all entries in set.
    FD_ZERO (&read_fds);// Clear all entries in set.
    int maxfd;
    
    struct addrinfo hints;
    struct addrinfo *servinfo, *p;
    struct sockaddr_storage clientinfo, tempinfo;
    
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // TCP stream sockets
    
    //get address info from command line
    if ((status = getaddrinfo(argv[1], argv[2] , &hints, &servinfo)) != 0) {
        cerr<< "getaddrinfo error: %s\n"<< gai_strerror(status) << endl;
        exit(0);
    }
    //bind
    for(p=servinfo; p!=NULL; p=p->ai_next){
        if((servsock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("linstener: socket");
            continue;
        }
        cout << "Socket established." << endl;
        
        if ((bind(servsock, p->ai_addr, p->ai_addrlen)) == -1) {
            close(servsock);
            perror("listener: bind");
            continue;
        }
        break;
    }
    
    if(p==NULL){
		fprintf(stderr, "my sock fd: failed to bind\n");
		return 2;
	}
    
    cout << "Socket binded." << endl;
    
    cout << "Waiting to recvfrom ..." << endl;
    
    //initialize variables for recvfrom
    char buff[buffer_size];
    int bytes;
    
    struct sockaddr newaddr;
    newaddr = *(p->ai_addr);
    struct sockaddr_in* new_address;
    new_address = (struct sockaddr_in*) &newaddr;
    new_address->sin_port = htons(0);
    
    //sa.sa_handler = sigchld_handler; // kill all dead processes
    // sigemptyset(&sa.sa_mask);
    // sa.sa_flags = SA_RESTART;
    // if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    //     perror("sigaction");
    //     exit(1);
    // }
    
    while(true){
        //call recvfrom to get a RRQ from client
        
        
        
    }
    
    
    exit(1);
}