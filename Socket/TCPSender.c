#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

void dieWithMessage(char * msg)
{
    printf("%s\n", msg);
    exit(0);
}

#define PEER_COUNT 2
int main(int argc, char *argv[])
{
    char buffer[BUFSIZE];
    int *peerFds = NULL;

    peerFds = (int *)malloc(sizeof(int) * PEER_COUNT);

    // Create socket for incoming connections
    int servSock; // Socket descriptor for server
    if((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        dieWithMessage("socket() failed\n");

    //Construct local address structure
    struct sockaddr_in servAddr;                    // Local address
    memset(&servAddr, 0 , sizeof(servAddr));        // Zero out structure
    servAddr.sin_family = AF_INET;                  // IPv4 address family
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);   // Any incoming interface
    servAddr.sin_port = htons(12345);               // Local port

    // Bind to the local address
    if(bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
        dieWithMessage("bind() failed");

    // Mark the socket so it will listen for incoming connections
    if(listen(servSock, PEER_COUNT) < 0)
        dieWithMessage("listen() failed");

    for (int i=0; i < PEER_COUNT; i++)
    {
        struct sockaddr_in clntAddr; // Client address
        // Set length of client address structure (in-out parameter)
        socklen_t clntAddrLen = sizeof(clntAddr);
        // Wait for a client to connect
        peerFds[i] = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
        if (peerFds[i] < 0)
            dieWithMessage("accept() failed");
    }

    for (int i=0; i < PEER_COUNT; i++)
    {
        ssize_t numBytesSent = send(peerFds[i], "Hello", 6, 0);
        if (numBytesSent < 0)
            dieWithMessage("send() failed");
    }

    free(peerFds);

    // NOT REACHED
    return 0;
}
