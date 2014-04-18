#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory.h>
#include <sys/time.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char g_myID[3];
//pthread_mutex_t g_lock_lc;
unsigned long g_lc;
pthread_t thread_id;
int *g_peerFds = NULL;
int g_maxFd, g_peerCount;

FILE *g_logfile = NULL;

#define BUFSIZE 300

char* GetOffset();
void init (char** argv);

void dieWithMessage(const char* msg)
{
    printf("%s\n", msg);
    exit(0);
}

__uint64_t getCurrentPhysicalTime()
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0)
        assert(!"gettimeofday failed!");

    return tv.tv_sec * 1000000 + tv.tv_usec;
}

class ATTime
{
    public:
    __uint64_t mLogicalTime;
    __uint64_t mLogicalCount;
    __uint64_t mPhysicalTime;

    ATTime()
    {
        mLogicalTime = 0;
        mLogicalCount = 0;
        mPhysicalTime = getCurrentPhysicalTime();
    }

    void createSendEvent(); 
    void copyClock(ATTime *src);
};

// global time
ATTime g_attime;

void writeState(FILE *fp, int type, char *recvString = NULL)
{
    char *offset = GetOffset();

    switch(type)
    {
        case 0: // send event
            fprintf (fp, "Send:");
            fprintf (fp, "%s:%lu:[%lu]:%lu:%s\n",g_myID, g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime, offset);
            //fprintf (fp, "%s:%lu:%lu:%lu\n",g_myID, g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime);
            break;
        case 1: // recv event
            fprintf (fp, "Recv:");
            fprintf (fp, "%s:%lu:[%lu]:%lu",g_myID, g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime);
            //fprintf (fp, ":%s:%s\n", offset, recvString);
            fprintf (fp, ":%s\n",  recvString);
            break;

        default:
            break;
    }

    free(offset);
}

void ATTime::copyClock(ATTime *src)
{
    mLogicalTime = src->mLogicalTime;
    mLogicalCount = src->mLogicalCount;
    mPhysicalTime = src->mPhysicalTime;
}

void ATTime::createSendEvent()
{
    ATTime *e = &g_attime;
    ATTime *f = new ATTime();

    f->mLogicalTime = std::max(e->mLogicalTime, f->mPhysicalTime);
    if (f->mLogicalTime == e->mLogicalTime)
    {
        f->mLogicalCount = e->mLogicalCount + 1;
    }
    else
    {
        f->mLogicalCount = 0;
    }

    g_attime.copyClock(f);
    writeState(g_logfile, 0);
    
    delete f;
}


char* GetOffset()
{
	FILE *fp;
  int status;
  char path[1035];
	char *ret = (char*)malloc(40);

  /* Open the command for reading. */
  fp = popen("ntpdc -cloopinfo | grep offset", "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  FILE* fp1 = popen("ntpdc -ckerninfo | grep offset", "r");
  if (fp1 == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }
  
  char path1[1035];
  fgets(path1, sizeof(path)-1, fp1);
 strtok(path1, ":");
        char *plloffsets = strtok(NULL, ":");
        char *plloffset = strtok(plloffsets, " ");
  /* Read the output a line at a time - output it. */
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
	strtok(path, ":");    
	char *offsets = strtok(NULL, ":");
	char *offset = strtok(offsets, " ");
        sprintf(ret, "%s|%s", offset,plloffset);
  }

  /* close */
  pclose(fp);
  pclose(fp1);
	return ret;
}

void init (char* argv[])
{
    int i, err, numBytes, retryCount;
    in_port_t remotePort;
    struct sockaddr_in remoteAddr;
    char *strIp, *strPort;

    char buffer[BUFSIZE];

    if (argv == NULL)
        return;

    g_peerFds = (int *)malloc(g_peerCount * sizeof(int));
    if (g_peerFds == NULL)
        dieWithMessage("memory too low");

    g_maxFd = 0;

    for (i = 0; i < g_peerCount; i++)
    {
        retryCount = 100;

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

        g_peerFds[i] = socket(AF_INET, SOCK_STREAM, 0);
        while((connect(g_peerFds[i], (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) < 0) && (retryCount > 0))
        {
            sleep(1);
            retryCount--;
        }

        if (retryCount == 0)
            dieWithMessage("Connect failed");
        printf("Connected to %s\n", strIp);
        g_maxFd = (g_peerFds[i] > g_maxFd) ? g_peerFds[i] : g_maxFd;
    }
}

int main (int argc, char* argv[])
{
    char *filename = (char *)"dump.log";

    if(argc < 3)
    {    
        printf("\nUSAGE: peer myID <peer1> [<peer2> .......]\n");
        exit(1);
    }

    g_logfile = fopen("events.log", "w");
    assert (g_logfile != NULL);

    //set current peer's ID in myID
    sprintf(g_myID, "%s", argv[1]);

    g_peerCount = 1;

    int *sendFds = NULL;

    sendFds = (int *)malloc(sizeof(int) * g_peerCount);

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
    if(listen(servSock, 1) < 0)
        dieWithMessage("listen() failed");

    {
        struct sockaddr_in clntAddr; // Client address
        // Set length of client address structure (in-out parameter)
        socklen_t clntAddrLen = sizeof(clntAddr);
        // Wait for a client to connect
        sendFds[0] = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
        if (sendFds[0] < 0)
            dieWithMessage("accept() failed");
    }

    printf ("Accepted connections from all peers\n");

    //Send Logic starts
    char message[300];
    char *messageHead = NULL;
    int sleepTime = 0;
    int bytesRem;
    int bytesSent;
    while (1)
    {
            g_attime.createSendEvent();
            char *offset = GetOffset();
            sprintf(message, "%s:%ld:%ld:%ld:%s", g_myID, g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime, offset);
            free(offset);
            //sprintf(message, "%s:%ld:%ld:%ld", g_myID, g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime);

            bytesRem = 300;
            messageHead = message;
            while (bytesRem)
            {
                bytesSent = send(sendFds[0], messageHead, 300, 0);
                bytesRem -= bytesSent;
                messageHead += bytesSent;
            }
        //usleep(250000);
        //sleepTime = rand() % 5;
        //sleep(sleepTime);
    }

    return 0;
}