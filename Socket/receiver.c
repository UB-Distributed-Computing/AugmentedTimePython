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
unsigned long g_lc;
int *g_peerFds = NULL;
int g_maxFd, g_peerCount=3;

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

    void createRecvEvent(__uint64_t msgLogicalTime, __uint64_t msgLogicalCount, __uint64_t msgPhysicalTime, char *recvString);
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

void ATTime::createRecvEvent(__uint64_t msgLogicalTime, __uint64_t msgLogicalCount, __uint64_t msgPhysicalTime, char *recvString)
{
    ATTime *e = &g_attime;
    ATTime *f = new ATTime(); // f physical time is up-to-date

    f->mLogicalTime = std::max(e->mLogicalTime, std::max(msgLogicalTime, f->mPhysicalTime));

    if ((f->mLogicalTime == e->mLogicalTime) && (f->mLogicalTime == msgLogicalTime))
    {
        f->mLogicalCount = std::max(e->mLogicalCount, msgLogicalCount) + 1;
    }
    else if (f->mLogicalTime == e->mLogicalTime)
    {
        f->mLogicalCount = e->mLogicalCount + 1;
    }
    else if (f->mLogicalTime == msgLogicalTime)
    {
        f->mLogicalCount = msgLogicalCount + 1;
    }
    else
    {
        f->mLogicalCount = 0;
    }

    g_attime.copyClock(f);
    writeState(g_logfile, 1, recvString);

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

void* Receiver(void* dummy)
{
    fd_set rfds;
    int i, err, bytesRecvd, bytesRem;

    char buffer [BUFSIZE];
    char buffercopy[BUFSIZE];
    char *bufferHead = NULL;

    init((char **)dummy);

    while (1)
    {
        FD_ZERO(&rfds);

        for(i=0; i < g_peerCount; i++)
        {
            FD_SET(g_peerFds[i], &rfds);
        }
        err = select(g_maxFd + 1, &rfds, NULL, NULL, NULL);

        if (err == -1)
        {
            dieWithMessage("select() failed");
        }
        else if (err)
        {
            for (i = 0; i < g_peerCount; i++)
            {
                if(FD_ISSET(g_peerFds[i], &rfds))
                {
                    bufferHead = buffer;
                    bytesRem = BUFSIZE;
                    while(bytesRem)
                    {
                        bytesRecvd = recv(g_peerFds[i], bufferHead, BUFSIZE, 0);
                        if(bytesRecvd < 0)
                            dieWithMessage("recv() failed");
                        else if(bytesRecvd == 0)
                            dieWithMessage("recv() Connection closed prematurely");

                        bytesRem -= bytesRecvd;
                        bufferHead += bytesRecvd;
                    }

                    strcpy(buffercopy, buffer);
                    char * chClient = strtok(buffer, ":");
                    char * strLogClk = strtok(NULL,":");
                    char * strLogCnt = strtok(NULL,":");
                    char * strPhyTime = strtok(NULL,":");

                    __uint64_t LogClk = strtol(strLogClk,NULL,10);
                    __uint64_t LogCnt = strtol(strLogCnt,NULL,10);
                    __uint64_t PhyTime = strtol(strPhyTime,NULL,10);

                    g_attime.createRecvEvent(LogClk, LogCnt, PhyTime, buffercopy);
                }
            }
        }
        else
        {
            dieWithMessage("Select timedout!");
        }
    }
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



    //spawn the receiver
   // int err = pthread_create(&thread_id, NULL, &Receiver, (void*)&argv[2]);
    //if (err != 0)
    //{
     //   printf("\ncan't create thread :[%s]", strerror(err));
      //  return 1;
   // }

    Receiver((void*)&argv[2]);

    return 0;
}
