#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// includes

// defines
#define BUFSIZE 1024
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

int main(int argc, char* argv[])
{
    if(argc<3 || argc>4)
        dieWithUserMessage("Usage", " TCPEchoClient server echo <port>");

    char *serverIp = argv[1];
    char *echoString = argv[2];
    //Third argument is optional
    in_port_t servPort = (argc==4)?atoi(argv[3]):7;

    //Creating reliable stream socket using TCP
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock<0)
        dieWithSystemMessage("Socket creation failed");

    struct sockaddr_in servAddr; //Server address
    memset(&servAddr, 0, sizeof(servAddr)); //zero out the structure
    servAddr.sin_family = AF_INET; //IPv4 address family
    int rtnVal = inet_pton(AF_INET, serverIp, &servAddr.sin_addr.s_addr);
    if(rtnVal == 0)
        dieWithUserMessage("inet_pton() failed", " Invalid address string");
    else if(rtnVal < 0)
        dieWithSystemMessage("inet_pton() failed");
    servAddr.sin_port = htons(servPort); //Server port

    if(connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
        dieWithSystemMessage("connect() failed");

    size_t echoStringLen = strlen(echoString); //Determine the input length

    //Send the string to the server
    ssize_t numBytes = send(sock, echoString, echoStringLen, 0);
    if(numBytes < 0)
        dieWithSystemMessage("send() failed");
    else if(numBytes != echoStringLen)
        dieWithUserMessage("send()", " Unexpected number of bytes were sent");
    
    //Recieve the same string back from server
    unsigned int totalBytesRcvd = 0; //Count the total bytes
    while(totalBytesRcvd < echoStringLen)
    {
        char buffer[BUFSIZE];

        //Recieve upto BUFSIZE-1 bytes from the sender
        numBytes = recv(sock, buffer, BUFSIZE-1, 0);
        if(numBytes < 0)
            dieWithSystemMessage("recv() failed");
        else if(numBytes == 0)
            dieWithUserMessage("recv()", " Connection closed prematurely");

        totalBytesRcvd += numBytes; //Keep tally of total bytes recieved
        buffer[numBytes] = '\0'; //Terminate the string
        fputs(buffer, stdout); //Print the buffer
    }

    fputs("\n", stdout); //Print final line feed

    close(sock);
    return 0;
}
