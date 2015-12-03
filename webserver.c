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

static int const ssthresh = 5; // FIXME: Do we have to change this?
static int const timeOut = 5;

bool test=false;


typedef struct window{
  int startIndex; // the start of the window.
  int windowLength; // the sender congestion window size
  int nextToSend; // the first 0 receive
  int segmentCount;	//number of segments
  int* acked; // 0: not yet sent; 1: sent but not yet acked; 2: sent
  char** segments;
  int endRTTCommand;
  int* timer; // keep track the remaining time for each packet before the timeout happens
  int* resendCommand ;
  // each time the prepareForSend function will update this property, when this ack sequence has been received by the sender, it indicate an RTT has been finished.
  // This property only matters when the congestionAvoidence is finished.


}* window_t;

window_t window;

bool isFinished();

//ANDREW CHANGE #1: Combined the prepareFile() and window() functions into one
void makeWindow(FILE* file){
  //TODO: Andrew
  // chunk the file into 984 bytes each and and 16 bytes header: the order will be: sequence#[4 bytes], fileSize[8 bytes], length[4 bytes], payload[984 bytes]
  // create window based on chunks
	//PART 1: Parsing file into Char Segments
  size_t PAYLOAD;
	if(test==false){
		PAYLOAD = 984;			//testing with small payload
		//PAYLOAD = 10;
	}
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
	window = malloc (sizeof(int)*5 + sizeof(int*)*3 + sizeof(char**));	//allocate memory for widow
	window->acked = malloc(sizeof(int)*segmentCount);
        for (i=0; i < segmentCount; i++)
		window->acked[i] = 0;

// Victor: additional setting in the window
  window->timer = malloc(sizeof(int)*segmentCount); // init timer array
  for(i = 0; i< segmentCount; i++){
    window->timer[i] = timeOut;
  }
  window->resendCommand = malloc(sizeof(int)); // single element resending command
  window->resendCommand[0] = -1;

	window->segments = segments;
	window->startIndex = 0; // the start of the window.
  	window->windowLength = 1; // starts off at 1
 	window->nextToSend = 0; // the first 0 receive
	window->segmentCount = segmentCount; // the first 0 receive
	printf( "makeWindow() constructor\n-------------\nfileSource: %s\nStart Index: %d\nSegmentCount: %d\ncmwd: %d\n\n\n",source, window->startIndex,window->segmentCount,window->windowLength );
	free(source);
	//return win;


}

void printWindow(){
	printf("\nprintWindow()\n----------\n");
	printf("Seq# | State | cmwd | next | Segment Data\n");
	printf("-------------------------------\n");
	int i;
	for ( i=0; i < window->segmentCount; i++){				//for all segments
		if ((i==window->startIndex || i==window->startIndex + window->windowLength-1) && i==window->nextToSend)
			printf("%04d | --%d-- |******|  XX  | %s \n", i, window->acked[i], window->segments[i]+16);
		else if (i==window->startIndex || i==window->startIndex + window->windowLength-1)
			printf("%04d | --%d-- |******|      | %s \n", i, window->acked[i], window->segments[i]+16);
		else if (i==window->nextToSend)
			printf("%04d | --%d-- |      |  XX  | %s \n", i, window->acked[i], window->segments[i]+16);
		else
			printf("%04d | --%d-- |      |      | %s \n", i, window->acked[i], window->segments[i]+16);

	}
}

void updateOnAcked(int ack){
  // TODO: Victor
  // Consider the situation when in the end of transfer
  // Not checking if the receiving ack is corrupted or not, assume all the packet it received is corrected
	printf("\n-----------\nReceiving ACK\n-----------\n");
	printf("ACK : %d\n", ack);
  	window->acked[ack] = 2;
  // Check the threshhold
  if(isFinished()==false)//ssthresh >= window->windowLength)
  {
    // slow start
	printf("Adopting Slow Start Algorithm, Current Window Size: %d \n",window->windowLength);
	if (window->startIndex + window->windowLength < window->segmentCount ){			//if the windowLength can be increased
		window->windowLength++;
		printf("New window size : %d\n", window->windowLength);
   	} 
	else{
		printf("Window size cannot be increased. At end of sequence array\n ");
	}
  }
  else{
	if (window->endRTTCommand == ack){
      // congestion avoidence.
		printf("Adopting Congestion Avoidence Algorithm, Current Window Size: %d \n",window->windowLength);
		if (window->startIndex + window->windowLength < window->segmentCount ){	
			window->windowLength++;
			printf("New window size : %d\n", window->windowLength);
		} 
     	else{
		printf("Window size cannot be increased. At end of sequence array\n ");
  	}
  }
    // keep using the slow start, update all the window properties
    // update startIndex:
  while(window->acked[window->startIndex] == 2)
  {
	if (window->startIndex  + window->windowLength < window->segmentCount ){
    		window->startIndex++;
		  if ( window->acked[window->startIndex+1] != 2)
		  	printf("New start index : %d\n", window->startIndex);
	}
	else {
		printf("Start index cannot be increased. At end of sequence array\n");
		break;
	}
  }

  while(window->acked[window->nextToSend] !=0){
	if (window -> nextToSend + 1 < window->segmentCount){
    		window->nextToSend ++;
		printf("New next to send : %d\n\n", window->nextToSend);
	}
	else{
		printf("No more segments to send \n\n");
		break;
	}
  }
   
}
}

void timeOutHandler(int signum){
  // TODO: Victor
  //FIXME: the signal handler function cannot have additional parameter put in, it can only deal with global variable. Might cause problem:
  int i;
  for(i = window->startIndex; i<= window->startIndex + window->windowLength; i++){
    window->timer[i]--;
    if (window->timer[i] == 0){
      printf("timeout happens at packet %d \n", window->timer[i]);
      window->resendCommand[0] = i;
      //FIXME; call the resend function the resendCommand.
    }
  }

  alarm(1);
}

int* prepareToSend(int* commandLength){
  // TODO: Victor
  // Consider the situation when in the end of transfer
  // calculate nums of the packet sending in this round first, then allocate the new packet.
  
  int size  = window->startIndex + window->windowLength - window->nextToSend;
	printf("\n-----------\nSending Segment\n-----------\n");
	printf("Size to send : %d\n", size);

  if (size <= 0){
    printf("no sending anything this round\n");
    return NULL;
  }
  int* command;
  command = (int *) malloc(size*sizeof(int));
  // put the command index in the command array.
  printf("Next to send : %d\n", window->nextToSend);
  int i = 0;
  int count = 0;
  while(i < size){
    //command[i] = window->acked[window->nextToSend+i];
  	 if (window->acked[window->nextToSend+i]==0){
		 command[count] = window->nextToSend+i;
		 count++;
		 i++;
	}
	else {
		i++;

	}
   
    
  }
    *commandLength = count;	//record the size of the command
	
  return command;
}

void sendPacket(int* command, int commandLength, int sock, const struct sockaddr* cli_addr, socklen_t clilen){
  // TODO: Andrew
  // send pack according to command
  // udpate the window after we sent
    //int* ptr = command;

    int i; 
    for( i = 0; i < commandLength; i++){
	int j = command[i];
	 printf("Sending segment : %d\n", command[i]);
	sendto(sock,window->segments[j],1000,0,cli_addr,(socklen_t) clilen);
  	window->acked[j]=1;
	if (window->nextToSend == window->segmentCount-1)
		window->nextToSend++;
	//ptr++;

    }

}

FILE* findFile (char* c){
  // TODO : Andrew
  // find file with the given file name and return the file pointer

  FILE *f = fopen(c, "r");
  if (f==NULL){
     fprintf(stderr,"Problem finding file");
  }
  printf( "findFile()\n----------\nFile: [%s]\n\n\n", c);

  return f;
}

bool isFinished(){
	if (window->startIndex + window->windowLength < window->segmentCount -1)
		return false;								//not finished yet
	int i;
	for (i=0; i< window->windowLength; i++){
		if (window->acked[window->startIndex+i] < 2)
			return false;
	}
	return true;
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
    signal(SIGALRM, timeOutHandler); // bind the timeOutHandler with the timer object.
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

    //window_t server_window = window(requestFile); //ANDREW CHANGE #4: Combined the prepareFile() and window() functions into one

	makeWindow(requestFile);		//allocate and construct window
	//if(test==true){
	if(1){
	  printWindow();
	}

    //FIXME: Not sure if anything missing
	
	
    int commandLength;	//single pointer to a single integer that saves the length of lastCommand()
    int* lastCommand = prepareToSend(&commandLength);
	printf( "Preparing to send\n{ ");
	 int p=0;
	for (p=0; p < commandLength; p++){
		if (p == commandLength - 1)
			printf( "%d }\n",lastCommand[p]);
		else
			printf( "%d, ",lastCommand[p]);
	}
	sendPacket(lastCommand, commandLength, sockfd, (struct sockaddr*)&cli_addr, clilen); // first send
        // TODO: free the lastCommand;

        free(lastCommand);
	commandLength=0;
	printWindow();
		
    alarm(1); // start timming cycle.
	
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
	ack = atoi(buffer);
        updateOnAcked(ack);

	 printWindow();
	if (isFinished()){
		printf( "Send complete! \n\n");
		break;
	}
	 
        lastCommand = prepareToSend(&commandLength);
	if(commandLength>0){
		printf( "Preparing to send:\n{ ");
		 int p=0;
		for (p=0; p < commandLength; p++){
			if (p == commandLength - 1)
				printf( "%d }\n",lastCommand[p]);
			else
				printf( "%d, ",lastCommand[p]);
		}
		sendPacket(lastCommand, commandLength, sockfd, (struct sockaddr*)&cli_addr, clilen); // first send
        	// TODO: free the lastCommand;
        	
   	}
	 else
		printf( "Nothing to Send \n\n");
	 printWindow();
	free(lastCommand);
	 commandLength=0;
        //TODO: free the lastCommand
        // free(lastCommand);
      }
    }
    return 0; /* we never get here */
}
