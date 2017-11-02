#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
//Network header file
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define buffer_size 512
#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5

using namespace std;
using std::ios;

//define TFTP structure
struct TFTP{
    uint16_t opcode;                            // 2 Bytes Opcode
    char * filename;                            // Filename
    char * mode;                                // Mode (Octet)
    uint16_t block_number;                      // Block Number
};

void handle_sigchld(int sig) {
  int saved_errno = errno;
  while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
  errno = saved_errno;
}
// void sigchld_handler(int s)
// {
//     while(waitpid(-1, NULL, WNOHANG) > 0);
// }


//convert from the host order to network byte order (16bit)
void convert_host_order (char *buf, unsigned short int i){   
	i = htons(i);
	memcpy(buf,&i,2);
}

//vonver from network byte order to the host order (16bit)
unsigned short int convert_network_byte(char *buf){	
	unsigned short int i;
	memcpy(&i,buf,2);
	i = ntohs(i);
	return i;
}

// get sockaddr, IPv4 or IPv6 
void *get_addr(struct sockaddr *address){
	if (address->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)address)->sin_addr);
	}
	return &(((struct sockaddr_in6*)address)->sin6_addr);
}

//convert packet to tftp structure
struct TFTP *read_packet(char *packet){
    struct TFTP *res=(struct TFTP*)malloc(sizeof(struct TFTP));
    res->opcode = convert_network_byte(packet);
    if(res->opcode == ACK){
        res->block_number=convert_network_byte(packet+2);
    }
    if(res->opcode == RRQ){
        int i=2;
        int j=0;
        char temp[512];
        while(packet[i]!='\0'){
            temp[j]=packet[i];
            i++;
            j++;
        }
        temp[j]='\0';
        res->filename=(char *)malloc(strlen(temp)*sizeof(char));
        strcpy(res->filename,temp);
        while(packet[i]=='\0') i++;
        j=0;
        while(packet[i]!='\0'){
            temp[j]=packet[i];
            i++;
            j++;
        }
        temp[j]='\0';
        res->mode=(char *)malloc(strlen(temp)*sizeof(char));
        strcpy(res->mode,temp);
    }
    return res;
}

//convert TFTP to charArray
char *to_tftp(uint16_t opcode, uint16_t block_number, char *message, int length){
    char *array;
    if(opcode == ERROR){
        array=(char *)malloc((length+5)*sizeof(char));
        
        convert_host_order(array,opcode);
        convert_host_order(array+2,block_number);
        memcpy(array+4,message,length);
        memset(array+4+length,'\0',1);
    }
    if(opcode==DATA){
        array=(char *)malloc((length+4)*sizeof(char));
        convert_host_order(array,opcode);
        convert_host_order(array+2,block_number);
        memcpy(array+4,message,length);
    }
    return array;
}

//get address regardless of ipv4 or ipv6

//generates random port between 1024 and 65535
unsigned int generate_ephemeral_port() 
{
	unsigned short int value;
	value = (rand()%64512) + 1024;
	return value;
}

int main(int argc, char *argv[]){
    
    if(argc != 3){ //if input argument is not exactly 3, return an error
        cerr << "usage: server ip port" << endl;  //cerr is cpp's error handler
        exit(0);
    }
    
    //declare variables and data structures
    int status; 
    int serv_sock; //server socket
    int option = 1;
    
    struct sigaction sa;
    
    fd_set master;// Create a master set of file discriptor.
    fd_set read_fds;  // temp file descriptor list for select()
    FD_ZERO (&master);// Clear all entries in set.
    FD_ZERO (&read_fds);// Clear all entries in set.
    int maxfd;
    
    socklen_t client_len;
    char s[INET_ADDRSTRLEN];
    
    struct addrinfo hints;
    struct addrinfo *serv_info, *p;
    struct sockaddr_storage client_addr;
    
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // TCP stream sockets
    
    //get address info from command line
    if ((status = getaddrinfo(argv[1], argv[2] , &hints, &serv_info)) != 0) {
        cerr<< "getaddrinfo error: %s\n"<< gai_strerror(status) << endl;
        exit(0);
    }
    //bind server socket to port
    for(p=serv_info; p!=NULL; p=p->ai_next){
        if((serv_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket");
            continue;
        }
        cout << "Socket established." << endl;
        
        setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

        if ((bind(serv_sock, p->ai_addr, p->ai_addrlen)) == -1) {
            perror("server: bind");
            continue;
        }
        break;
    }
    
    if(p==NULL){
		fprintf(stderr, "my sock fd: failed to bind\n");
		return 2;
	}
    
    cout << "Socket binded." << endl;
    
    cout << "Server is now online ...\n" << endl;
    
    //initialize variables for recvfrom
    char buff[buffer_size];
    int bytes;
    
    struct sockaddr new_addr;
    new_addr = *(p->ai_addr);
    struct sockaddr_in* new_address;
    new_address = (struct sockaddr_in*) &new_addr;
    new_address->sin_port = htons(generate_ephemeral_port());
    //new_address->sin_port = htons(0);
    
    sa.sa_handler = handle_sigchld; // kill all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    while(true){
        //call recvfrom to get a RRQ from client
        client_len = sizeof client_addr;
        if((bytes = recvfrom(serv_sock, buff, buffer_size - 1, 0, (struct sockaddr *)&client_addr, &client_len)) == -1){
            perror("recvfrom");
            exit(1);
        }
        struct sockaddr_in* new_client_addr = (struct sockaddr_in*)&client_addr;
        cout << "Server: got packet from IP address: " << 
        inet_ntop(client_addr.ss_family,get_addr((struct sockaddr *)&client_addr),s, sizeof s) << " "
        << "Port: " << ntohs(new_client_addr->sin_port) << endl;
        
        cout << "Server: packet is "<< bytes << " bytes long." << endl;
        
        struct TFTP *send_packet;
        send_packet = read_packet(buff);
        
        if(send_packet->opcode != RRQ && send_packet->opcode != WRQ){
            cout << "Request from client is invalid." << endl;
            continue;
        }
        
        if(!fork()){
            close(serv_sock); //close current server socket then create new server socket

            //create ephemeral socket to transfer files with client.
            if((serv_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
                perror("server: socket");
            }
        
            setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
            
            socklen_t new_len = sizeof new_addr;
            
            if ((bind(serv_sock, &new_addr, sizeof(new_addr))) == -1) {
                perror("server: bind");
            }
            
            if(getsockname(serv_sock, &new_addr, &new_len) == -1){
                perror("getsockname");
            }
            
            //open file which would be transmitted
            ifstream f;
            char *file_name = send_packet->filename;
            f.open(file_name, ios::in);
            if(!f.is_open()){
                //---------this might have issues-------//
                cout << "Could not find requested file: " << send_packet -> filename << endl;
                cerr << "Error: " << strerror(errno) << "\n" <<endl;
                string str = "Can not open requested file.";
                char *err_msg = (char *)malloc(512*sizeof(char));
                err_msg = strcpy(err_msg, str.c_str());
                int err_len = strlen(err_msg);
                const char *error_packet = to_tftp(ERROR, 1, err_msg, err_len);
                free(err_msg);
                if((bytes = sendto(serv_sock,error_packet,err_len + 5, 0, (struct sockaddr *)&client_addr,client_len)) == -1 ){
                        perror("server: sendto");
                        //exit(0);
                        continue; //modified might have issues
                }
                //------------end--------------//
                exit(0);
            }
            
            //calculate the file size
            streampos start, end;
            start = f.tellg();
            f.seekg(0, ios::end);
            end = f.tellg();
            f.seekg(0, ios::beg);
            int file_size = end - start;
            int packet_num = file_size/512 + 1;
            
            cout << "The ephemeral port for this transmission is " << ntohs(new_address->sin_port) << "." << endl;
            cout << "File size is: " << file_size << " bytes." << endl;
            cout << "Packet count is: " << packet_num << endl;
            
            char file_buf[buffer_size + 1];
            int block_number = 0;
            int last_ack = 0;
            int resend_count = 0;
            int f_len;
            
            FD_SET(serv_sock, &master);
            maxfd = serv_sock;
            struct timeval t;
            t.tv_sec = 1;
            //t.tv_usec = 50000;
            
            while(last_ack < packet_num){
                if(last_ack == block_number){
                    f.read(file_buf, buffer_size);
                    f_len = f.gcount(); //count the characters get from file
                    block_number++;
                    resend_count = 0;
                }
                
                if(resend_count > 0){
                    if(resend_count == 51){
                        cout << "Client Timeout." << endl;
                        break;
                    }
                    cout << "Try to resend packet." << endl;
                }
                
                char *packet = to_tftp(DATA, block_number, file_buf, f_len);  // convert data the TFTP packet structure
                
                if((bytes = sendto(serv_sock, packet, f_len + 4, 0, (struct sockaddr *)&client_addr,client_len)) == -1){
                    perror("Server: sendto");
                    exit(0);
                }
                free(packet);
                
                t.tv_sec = 1; //reset timer
                
                read_fds = master;
                
                //select to handle multiple clients connections
                if(select(maxfd + 1, &read_fds, NULL, NULL, &t) == -1){
                    perror("Server: select");
                    exit(0);
                }
                
                if(FD_ISSET(serv_sock, &read_fds)){
                    struct sockaddr_storage temp_addr;
                    socklen_t temp_len = sizeof temp_addr;
                    if((bytes = recvfrom(serv_sock, buff, 4, 0, (struct sockaddr *)&temp_addr, &temp_len)) == -1){
                        perror("recvfrom");
                        exit(0);
                    }
                }
                struct sockaddr_in* temp_client_addr = (struct sockaddr_in*)& client_addr;
                
                if(temp_client_addr->sin_addr.s_addr == new_client_addr->sin_addr.s_addr){
                    send_packet = read_packet(buff);
                    last_ack = send_packet->block_number;
                }
                resend_count ++;
            }//end of transfering packets
            f.close();
            close(serv_sock);
            cout << "File transmission complete.\n" << endl;
            exit(0);
        }
        free(send_packet);
    }
    
    close(serv_sock);
	freeaddrinfo(serv_info);
    exit(1);
}