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

int main(int argc, char *argv[])
{
    int i, err, numBytes, peerCount, maxFd;
    in_port_t remotePort;
    struct sockaddr_in remoteAddr;
    char *strIp, *strPort;

    int *peerFds = NULL;
    fd_set rfds;

    char buffer[BUFSIZE];

    if(argc < 2) // Test for correct number of arguments
        dieWithMessage("Usage: %s <server1:port1> [<server2:port2> ... ]");

    peerCount = argc - 1;
    peerFds = (int *)malloc(peerCount * sizeof(int));
    if (peerFds == NULL)
        dieWithMessage("memory too low");

    FD_ZERO(&rfds);
    maxFd = 0;

    for (i = 1; i <= peerCount; i++)
    {
        strIp = strtok(argv[i], ":");
        strPort = strtok(NULL, ":");

        remotePort = atoi(strPort);

        memset(&remoteAddr, 0, sizeof(remoteAddr));
        remoteAddr.sin_family = AF_INET;

        err = inet_pton(AF_INET, strIp, &remoteAddr.sin_addr.s_addr);
        if(err == 0)
            dieWithMessage("inet_pton() failed - Invalid address string");
        else if(err < 0)
            dieWithMessage("inet_pton() failed");
        remoteAddr.sin_port = htons(remotePort);

        peerFds[i-1] = socket(AF_INET, SOCK_STREAM, 0);
        if(connect(peerFds[i-1], (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) < 0)
            dieWithMessage("connect() failed");

        FD_SET(peerFds[i-1], &rfds);
        maxFd = (peerFds[i-1] > maxFd) ? peerFds[i-1] : maxFd;
    }

    while (1)
    {
        FD_ZERO(&rfds);
        for(i=0; i < peerCount; i++)
        {
            FD_SET(peerFds[i], &rfds);
        }
        err = select(maxFd + 1, &rfds, NULL, NULL, NULL);

        if (err == -1)
        {
            dieWithMessage("select() failed");
        }
        else if (err)
        {
            for (i = 0; i < peerCount; i++)
            {
                if(FD_ISSET(peerFds[i], &rfds))
                {
                    numBytes = recv(peerFds[i], buffer, BUFSIZE-1, 0);
                    if(numBytes < 0)
                        dieWithMessage("recv() failed");
                    else if(numBytes == 0)
                        dieWithMessage("recv() Connection closed prematurely");

                    buffer[numBytes] = '\0';

                    printf("%s\n", buffer);
                    //FD_CLR(peerFds[i], &rfds);
                }
            }
        }
        else
        {
            dieWithMessage("Select timedout!");
        }
    }

    free(peerFds);

    // NOT REACHED
    return 0;
}
