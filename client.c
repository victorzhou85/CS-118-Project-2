
/*
 A simple client in the internet domain using TCP
 Usage: ./client hostname port (./client 192.168.0.151 10000)
 */


#include "segment.h"
#include "sequence.h"


#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>      // define structures like hostent
#include <stdlib.h>
#include <strings.h>

const int MSS = 1000;

void doSomething( char* filename, int sock, struct sockaddr_in serv_addr);

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd; //Socket descriptor
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server; //contains tons of information, including the server's IP address
    char* filename;	

    
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port filename\n", argv[0]);
       exit(0);
    }
    
   
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); //create a new socket
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    server = gethostbyname(argv[1]); //takes a string like "www.yahoo.com", and returns a struct hostent which contains information, as IP address, address type, the length of the addresses...
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
     filename =  atoi(argv[3]);

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //initialize server's address
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    
    while(1) {
       doSomething(filename, sockfd, serv_addr);
       
    }

    close(sockfd); //close socket
    
    return 0;
}

void doSomething(char* filename, int sock, struct sockaddr_in serv_addr){
	
	int cwnd = 0;
	int fileSize = 0;
	char* save;
	sequence_t sequence;
	segment_t seg;
	char buffer[256];

	char* data;
	int seq;
	

	
	int n = sendto(sock, filename, strlen(filename), 0, (struct sockaddr_in *) &serv_addr, sizeof(serv_addr)); //write to the socket
        if (n < 0) 
             error("ERROR writing to socket");
        
	bool initialize, complete =  false;

	//WHILE FILE IS NOT FULLY TRANSFERRED. 
	while(!complete){
		//FIRST SEGMENT RECEIVED
		if (!initialize){
			bzero(buffer,256);
			read(sock,buffer,255);
			data = malloc(984);
			
			//RECORD FIELDS OF THE RECEIVED SEGMENT
			seg = charToSeg(buffer);
			cwnd = seg->cwnd;
			fileSize = seg->fileSize;
			seq = seg->seq;
			memcpy(data, seg->data, 984);
		
			//INITIALIZE SEQUENCE
			sequence= makeSequence( cwnd, fileSize/MSS );

			//FIXME 7: CONCATENATE char* data TO THE RIGHT LOCATION OF char* save

			sequence->value[seq]=true;
			advanceWindow(sequence);
			
			//FIXME 8: SEND ACK TO SENDER
			free(seg);
			free(data);
			initialize = true;	
		}
		else{	//ALL OTHER SEGMENTS
			bzero(buffer,256);
			read(sock,buffer,255);
			data = malloc(984);
			
			//RECORD FIELDS OF THE RECEIVED SEGMENT
			seg = charToSeg(buffer);
			seq = seg->seq;
			memcpy(data, seg->data, 984);

			//FIXME 9: CONCATENATE char* data TO THE RIGHT LOCATION OF char* save (SAME AS FIXME 7)

			sequence->value[seq]=true;
			advanceWindow(sequence);
			
			//FIXME 10: SEND ACK TO SENDER (SAME AS FIXME 8)

			free(seg);
			free(data);
				
		}
		if (sizeof(save) == fileSize){
			//FIXME 11: Send save to a file
			complete = true;
		}
		
	
	}


	

	


}
