/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection

   Modified by Gregory Liu :)

   Returns an HTML response with headers for a requested file
   Returns 404 on improperly requested file or bad file
*/

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
    // char blah[100];
    // char blah2[100];
    // char blah3[100];
    char* hi = "0000000500000013BEGIN";
    char* hi2 = "0001000500000013KAPPA";
    char* hi3 = "0002000300000013123";

    
    // char* ptr2 = hi2;
    // char* ptr3 = hi3;

    // char test[6];
    // char* ptr = test;
    // char test2[30];
    // char* ptr = test;
    // char* ptr2 = test2;
    
    
         // FILE* fp = fopen("a.txt", "r");

    // fread(test, 1, 5, fp);
    // test[5] = '\0';

    // strncpy(blah, hi, 17);
    // printf("hi: %s\n", blah );
    // strcat(blah, test);
    // memcpy(hi, test, 5);
    // fseek(fp, 5, SEEK_SET);
    // fread(ptr2 + 16, 1, 5, fp);
    // fseek(fp, 10, SEEK_SET);
    // fread(ptr3 + 16, 1, 3, fp);

    // fclose(fp);

    // printf("hi: %s\n", blah );
    // printf("test2: %s\n", hi2);
    // printf("test3: %s\n", hi3);
    
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
    
    // int i = 0;
    // while (i < 2) {
    int n;
    char buffer[256];
    
    bzero(buffer, 256);
    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr_in *) &cli_addr, (socklen_t *) &clilen);
    if (n < 0)
        error("ERROR reading from socket");
    else {
        printf("\njustreceived: ");
        fprintf(stdout, buffer);
        printf(" END\n");
    }

    bzero(buffer, 256);
    printf("hi: %s (that's all)\n", hi);
    n = sendto(sockfd,hi,strlen(hi), 0, (struct sockaddr_in*) &cli_addr, (socklen_t) clilen); //write to the socket
    if (n < 0) 
        error("ERROR writing to socket");
    

    // TWO
    bzero(buffer, 256);
    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr_in *) &cli_addr, (socklen_t *) &clilen);
    if (n < 0)
        error("ERROR reading from socket");
    else {
        printf("\njustreceived: ");
        fprintf(stdout, buffer);
        printf(" END\n");
    }

    bzero(buffer, 256);
    printf("hi2: %s (that's all)\n", hi2);
    n = sendto(sockfd,hi3,strlen(hi3), 0, (struct sockaddr_in*) &cli_addr, (socklen_t) clilen); //write to the socket
    if (n < 0) 
        error("ERROR writing to socket");

    // THREE

    bzero(buffer, 256);
    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr_in *) &cli_addr, (socklen_t *) &clilen);
    if (n < 0)
        error("ERROR reading from socket");
    else {
        printf("\njustreceived: ");
        fprintf(stdout, buffer);
        printf(" END\n");
    }

    bzero(buffer, 256);
    printf("hi3: %s (that's all)\n", hi3);
    n = sendto(sockfd,hi2,strlen(hi2), 0, (struct sockaddr_in*) &cli_addr, (socklen_t) clilen); //write to the socket
    if (n < 0) 
        error("ERROR writing to socket");

    bzero(buffer, 256);
    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr_in *) &cli_addr, (socklen_t *) &clilen);
    if (n < 0)
        error("ERROR reading from socket");
    else {
        printf("\njustreceived: ");
        fprintf(stdout, buffer);
        printf(" END\n");
    }

    // } /* end of while */
    return 0; /* we never get here */
}