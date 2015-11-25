/* A simple server in the internet domain using TCP
   The port number is passed as an argument
   This version runs forever, forking off a separate
   process for each connection

   Modified by Gregory Liu :)

   Returns an HTML response with headers for a requested file
   Returns 404 on improperly requested file or bad file
*/
//#include "segment.h"
//#include "sequence.h"

typedef struct window{
  int startIndex; // the start of the window.
  int windowLength; // the sender congestion window size
  int nextToSend; // the first 0 receive
  int* acked; // 0: not yet sent; 1: sent but not yet acked; 2: sent
  char** segments;
}* window_t;

char** prepareFile(file *file){
  //TODO: Andrew
  // chunk the file into 984 bytes each and and 16 bytes header: the order will be: sequence#[4 bytes], fileSize[8 bytes], length[4 bytes], payload[984 bytes]
}

window_t window(char** segments){
  // TODO: Andrew
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

void sendPacket(window_t window, int* command){
  // TODO: Andrew
  // send pack according to command
  // udpate the window after we sent
}

file* findFile (char*){
  // TODO : Andrew
  // find file with the given file name and return the file pointer
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

    if (argc < 5) {
        fprintf(stderr,"ERROR, proper use: ./sender <port number> <cwnd> <p_loss> <p_corrupt>\n");
        exit(1);
    }

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

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    clilen = sizeof(cli_addr);

    file* requestFile; // FIXME: Read the document about allocate a new file in ram

    while (1) {
        int n;
        char buffer[256];

        bzero(buffer, 256);
        n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr_in *) &cli_addr, (socklen_t *) &clilen);
        if (n < 0)
            error("ERROR reading from socket");
        else {
            // fprintf(stdout, buffer);
            requestFile = findFile(buffer);
            break;
            // break out the initial round of request
            // FIXME: if the file is not found, send something to client
        }

        // bzero(buffer, 256);
        n = sendto(sockfd,hi,strlen(hi), 0, (struct sockaddr_in*) &cli_addr, (socklen_t) clilen); //write to the socket
        if (n < 0)
            error("ERROR writing to socket");
    } /* end of while */

    /* init window */

    window_t server_window = window(prepareFile(requestFile));
    //FIXME: Not sure if anything missing
    int* lastCommand = prepareToSend(server_window);
    sendPacket(server_window, lastCommand); // first send
    // TODO: free the lastCommand;
    while(1){
      // receving acks from recever:
      int n;
      char buffer[256]; // buffer for acks
      bzero(buffer, 256);
      n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr_in *) &cli_addr, (socklen_t *) &clilen);
      if (n < 0)
          error("ERROR reading from socket");
      else {
        int ack = 0;
        // TODO: parse the acks from buffer:
        updateOnAcked(window, ack);
        lastCommand = prepareToSend(server_window);
        sendPacket(server_window, lastCommand);
        //TODO: free the lastCommand
      }
    }
    return 0; /* we never get here */
}
