#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
//#include <DieWithMessage.h>
#define BUFSIZE 1024

static const int MAXPENDING = 5; // Maximum outstanding connection requests
void dieWithSystemMessage(char * msg)
{
    printf("%s", msg);
    exit(0);
}

void dieWithUserMessage(char * msg, char* msg1)
{
    printf("%s: %s", msg, msg1);
    exit(0);
}

__uint64_t getCurrentPhysicalTime()
{
    struct timeval tv;

    gettimeofday(&tv, NULL); 

    return tv.tv_sec * 1000000 + tv.tv_usec;
}


void HandleTCPClient(int clntSocket) 
{
    char buffer[BUFSIZE]; // Buffer for echo string

    // Receive message from client
    ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
    if (numBytesRcvd < 0)
        dieWithSystemMessage("recv() failed");

    // Send received string and receive again until end of stream
    while (numBytesRcvd > 0)  // 0 indicates end of stream
    {
        // Echo message back to client
        ssize_t numBytesSent = send(clntSocket, buffer, numBytesRcvd, 0);
        if (numBytesSent < 0)
            dieWithSystemMessage("send() failed");
        else if (numBytesSent != numBytesRcvd)
            dieWithUserMessage("send()", " sent unexpected number of bytes");
        // See if there is more data to receive
        numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
        if (numBytesRcvd < 0)
            dieWithSystemMessage("recv() failed");
    }
    close(clntSocket); // Close client socket
}

int main(int argc, char *argv[])
{
    if(argc != 2) // Test for correct number of arguments
        dieWithUserMessage("Usage:", " TCPEchoServer port\n");

    in_port_t servPort = atoi(argv[1]); // First argument - Local port

    // Create socket for incoming connections
    int servSock; // Socket descriptor for server
    if((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        dieWithSystemMessage("socket() failed\n");

    //Construct local address structure
    struct sockaddr_in servAddr;                    // Local address
    memset(&servAddr, 0 , sizeof(servAddr));        // Zero out structure
    servAddr.sin_family = AF_INET;                  // IPv4 address family
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);   // Any incoming interface
    servAddr.sin_port = htons(servPort);            // Local port

    // Bind to the local address
    if(bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
        dieWithSystemMessage("bind() failed");

    // Mark the socket so it will listen for incoming connections
    if(listen(servSock, MAXPENDING) < 0)
        dieWithSystemMessage("listen() failed");
	printf("Listening...\n");
    for(;;) // Run forever
    {
        struct sockaddr_in clntAddr; // Client address
        // Set length of client address structure (in-out parameter)
        socklen_t clntAddrLen = sizeof(clntAddr);
        // Wait for a client to connect
        int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
printf("Accepted\n");
        if (clntSock < 0)
            dieWithSystemMessage("accept() failed");
        // clntSock is connected to a client!
        char clntName[INET_ADDRSTRLEN]; // String to contain client address
        if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName, sizeof(clntName)) != NULL)
            printf("Handling client %s/%d\n", clntName, ntohs(clntAddr.sin_port));
        else
            puts("Unable to get client address");
        HandleTCPClient(clntSock);
    }

    // NOT REACHED
    return 0;
}
