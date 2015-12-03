/* A simple server in the internet domain using TCP
   The port number is passed as an argument
   This version runs forever, forking off a separate
   process for each connection

   Modified by Gregory Liu :)

   Returns an HTML response with headers for a requested file
   Returns 404 on improperly requested file or bad file
*/


typedef enum { false, true } bool;


//#include "segment.h"
//#include "sequence.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>

bool test=false;


typedef struct window{
  int startIndex; // the start of the window.
  int windowLength; // the sender congestion window size
  int nextToSend; // the first 0 receive
  int segmentCount;	//number of segments
  int* acked; // 0: not yet sent; 1: sent but not yet acked; 2: sent
  char** segments;
  
}* window_t;


//ANDREW CHANGE #1: Combined the prepareFile() and window() functions into one
window_t window(FILE* file){
  //TODO: Andrew
  // chunk the file into 984 bytes each and and 16 bytes header: the order will be: sequence#[4 bytes], fileSize[8 bytes], length[4 bytes], payload[984 bytes]
  // create window based on chunks



	//PART 1: Parsing file into Char Segments
	size_t PAYLOAD;
	if(test==false)
		PAYLOAD = 984;
	else
		PAYLOAD = 10;
	char *source = NULL; //create one string of the data which is to be parsed into an array of strings
	
	fseek(file, 0L, SEEK_END); //go to the end of the file
	int fileSize = ftell(file); //retrieve the size of the file
	source = malloc(sizeof(char)*(fileSize + 1)); //Allocate memory for the source String

	
	fseek(file, 0L, SEEK_SET); //go back to front of file
	

	size_t sourceLength = fread(source, sizeof(char), fileSize, file);	//read the file into source
	source[sourceLength] = '\0';
	

	char** segments;	
	size_t segmentCount = sourceLength/PAYLOAD; 	//retrieve the number of segments
	if ( sourceLength % PAYLOAD > 0){
		segmentCount += 1;			//adds another segment for an incomplete payload

	}
	segments = malloc(sizeof(char*)*segmentCount);	//allocate data for segment string
	int i;
	for ( i=0; i < segmentCount; i++){
		segments[i] = malloc(sizeof(char)*(PAYLOAD+16));		//allocate data for each individual segment
	}
	int count = 0;
	int iter = 0;
	int dataLen = 0;

	for ( i=0; i < segmentCount; i++){				//for all segments
		dataLen = PAYLOAD;					//len of data = PAYLOAD size
		if (sourceLength - iter < PAYLOAD)
			dataLen = sourceLength - iter;			
		sprintf(segments[i],"%04d",i);		//segment starts with sequence number
		sprintf(segments[i]+4,"%04d",dataLen);	//append the size of the payload
		sprintf(segments[i]+8,"%08d",fileSize);	//leave a gap for len and append fileSize
		while (count < PAYLOAD && iter < sourceLength){		//while the specific segement is less then payload and the iterator is less then entire file size
			segments[i][count+16] = source[iter];		//fill out the data
			count++;
			iter++;
		}
		segments[i][count+16] = '\0'; 
		count = 0;
		
	}

	

	//PART 2: Construct Window
	window_t win = malloc (sizeof(int)*4 + sizeof(int*) + sizeof(char**));	//allocate memory for widow
	win->acked = malloc(sizeof(int)*segmentCount);
        for (i=0; i < segmentCount; i++)
		win->acked[i] = 0;
	win->segments = segments;
	win->startIndex = 0; // the start of the window.
  	win->windowLength = 1; // starts off at 1
 	win->nextToSend = 0; // the first 0 receive
	win->segmentCount = segmentCount; // the first 0 receive
	printf( "window() constructor\n-------------\nfileSource: %s\nStart Index: %d\nSegmentCount: %d\ncmwd: %d\n\n\n",source, win->startIndex,win->segmentCount,win->windowLength );
	free(source);  	
	return win;


}

void printWindow(window_t window){
	printf("printWindow()\n----------\n");
	printf("Seq# | State | Segment Data\n");
	printf("---------------------------\n");
	int i;
	for ( i=0; i < window->segmentCount; i++){				//for all segments
		printf("%04d | --%d-- | %s\n", i, window->acked[i], window->segments[i]+16);	
	}
}

void updateOnAcked(window_t window, int ack){
  // TODO: Victor
  // Consider the situation when in the end of transfer
}

void timeOutHandler(window_t window){
  // TODO: Victor
  //
}

int* prepareToSend(window_t window){
  // TODO: Victor
  // Consider the situation when in the end of transfer
}

void sendPacket(window_t window, int* command, int sock, const struct sockaddr* cli_addr, socklen_t clilen){
  // TODO: Andrew
  // send pack according to command
  // udpate the window after we sent
    int* ptr = command;
    char* buffer;
    buffer = malloc(sizeof(char*) * 1000);	//allocate 1000 byte sized buffer
    while (ptr!=NULL){
	int i=*ptr;
	sendto(sock,window->segments[i],1000,0,cli_addr,(socklen_t) clilen);
  	window->acked[i]=1;
	ptr++;
	free(buffer);
    }
    
}

FILE* findFile (char* c){
  // TODO : Andrew
  // find file with the given file name and return the file pointer

  FILE *f = fopen(c, "r");
  if (f==NULL){
     fprintf(stderr,"Problem finding file");
  }
  printf( "findFile()\n----------\nFile: %s\n\n\n", c);

  return f;
}

#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h> /* for the waitpid() system call */
#include <sys/stat.h> /* for stat for last modified */
#include <signal.h> /* signal name macros, and the kill() prototype */

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void dostuff(int); /* function prototype */
int buildResponse(char* response, char* nextPiece, int offset); /* prototype */
char* getContentType(char* filename); /* prototype */
char* getRequestedFilepath(char* buffer); /* prototype */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    
    int sockfd, newsockfd, cwnd_length, portno, pid;
    float ploss, pcorr;
    char* tail;
    char* hi = "Received your message";
	
    socklen_t clilen;
   struct sockaddr_in serv_addr, cli_addr;		
   

    if (argc < 5 && argc!=2) {
        fprintf(stderr,"ERROR, proper use: ./sender <port number> <cwnd> <p_loss> <p_corrupt>\n");
        exit(1);
    }
    if (argc==2){
	test=true;
    }

  if(test==false){
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);
    cwnd_length = atoi(argv[2]);

    ploss = strtof(argv[3], &tail);
    pcorr = strtof(argv[4], &tail);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //ANDREW CHANGE #2: casted &serv_addr to (struct sockaddr *) instead of (struct sockaddr_in*)
        error("ERROR on binding");

    clilen = sizeof(cli_addr);
  }

    FILE* requestFile; // FIXME: Read the document about allocate a new file in ram


   char* testInput;
   
   if (test==false){		//if not a test, read socket input into buffer
	while (1) {
		int n;
        	char buffer[256];

       	 	bzero(buffer, 256);
       		 n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*) &cli_addr, (socklen_t *) &clilen); //ANDREW CHANGE #3: casted &cli_addr to (struct sockaddr *) 
        	if (n < 0)
            		error("ERROR reading from socket");
       		 else {
            		// fprintf(stdout, buffer);
           		 requestFile = findFile(buffer);
     
			
           		 break;
           		 // break out the initial round of request
            		// FIXME: if the file is not found, send something to client
       		 }

   	 } /* end of while */
   }
   else{		//if a test, read "test" into buffer
	 testInput = argv[1];
   	 printf( "\n\n\nmain()\n----------\nRequested Filename: %s\n\n\n",testInput);
	 requestFile = findFile(testInput);


   }

    /* init window */
	  
    //FIRST BURST OF COMMANDS

    window_t server_window = window(requestFile); //ANDREW CHANGE #4: Combined the prepareFile() and window() functions into one
	
	if(test==true){
	  printWindow(server_window);
	}

    //FIXME: Not sure if anything missing

    int* lastCommand = prepareToSend(server_window);
    sendPacket(server_window, lastCommand, sockfd, (struct sockaddr*)&cli_addr, clilen); // first send
    // TODO: free the lastCommand;
    free(lastCommand);
   
    while(1){
      // receving acks from recever:
      int n;
      char buffer[256]; // buffer for acks
      bzero(buffer, 256);
      n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cli_addr, (socklen_t *) &clilen);
      if (n < 0)
          error("ERROR reading from socket");
      else {
        int ack = 0;
        // TODO: parse the acks from buffer:
        updateOnAcked(server_window, ack);
        lastCommand = prepareToSend(server_window);
        sendPacket(server_window, lastCommand, sockfd, (struct sockaddr*) &cli_addr, clilen);
        //TODO: free the lastCommand
         free(lastCommand);
      }
    }
    return 0; /* we never get here */
}
