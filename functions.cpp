#include<iostream>
#include<fstream>
#include<string>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<signal.h>

#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5
using namespace std;

struct TFTP{
    uint16_t opcode;                            // 2 Bytes Opcode
    char * filename;                            // Filename
    char * mode;                                // Mode (Octet)
    uint16_t block_number;                             // Block Number
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

int main(int argc, char *argv[]){
    return 0;
}

