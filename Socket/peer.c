#include <stdio.h>
#include <errno.h>
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
pthread_mutex_t g_lock_lc;
unsigned long g_lc;
pthread_t thread_id;
int *g_peerFds = NULL;
int g_maxFd, g_peerCount;

FILE *g_logfile = NULL;

char *g_buffer = NULL;
unsigned g_msg_count = 0;

#define BUFSIZE 150
#define NUM_MESSAGES 1000
char* GetOffset();
void init (char** argv);
void dumpBufferToFile(FILE *fp);

void dieWithMessage(const char* msg)
{
    printf ("__%s:%d__\n", __func__, __LINE__);
    printf("%s\n", msg);
    exit(0);
}

__uint64_t getCurrentPhysicalTime()
{
    struct timeval tv;

    printf ("__%s:%d__\n", __func__, __LINE__);
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
    void createRecvEvent(__uint64_t msgLogicalTime, __uint64_t msgLogicalCount, __uint64_t msgPhysicalTime, char *recvString, ATTime* f);
    void copyClock(ATTime *src);
};

// global time
ATTime g_attime;

void dumpBufferToFile(FILE *fp)
{
    printf ("__%s:%d__\n", __func__, __LINE__);
    char *buf = g_buffer;
    printf("Writing from buffer %d message \n", g_msg_count);
    for (int msg_count = 0; msg_count < g_msg_count; msg_count++)
    {
        buf = g_buffer + msg_count * BUFSIZE;
        fprintf (fp, "%s", buf);
    }
    printf ("__%s:%d__\n", __func__, __LINE__);
    printf("Done writing from buffer %d message \n", g_msg_count);
}

void writeState(FILE *fp, int type, char *recvString = NULL)
{
    //char *offset = GetOffset();
    char *offset = (char*)"";

    char *buf = g_buffer + g_msg_count * BUFSIZE;

    printf ("__%s:%d__\n", __func__, __LINE__);
    switch(type)
    {
        case 0: // send event
    printf ("__%s:%d__\n", __func__, __LINE__);
            //fprintf (fp, "Send:");
            //fprintf (fp, "%s:%lu:[%lu]:%lu:%s\n",g_myID, g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime, offset);
            //fprintf (fp, "%s:%lu:%lu:%lu\n",g_myID, g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime);
            snprintf (buf, BUFSIZE, "Send:%s:%lu:[%lu]:%lu:%s\n",g_myID, g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime, offset);
            break;
        case 1: // recv event
    printf ("__%s:%d__\n", __func__, __LINE__);
            //fprintf (fp, "Recv:");
            //fprintf (fp, "%s:%lu:[%lu]:%lu",g_myID, g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime);
            //fprintf (fp, ":%s:%s\n", offset, recvString);
            //fprintf (fp, ":%s\n",  recvString);
            snprintf (buf, BUFSIZE, "Recv:%s:%lu:[%lu]:%lu:%s\n",g_myID, g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime, recvString);
            break;

        default:
    printf ("__%s:%d__\n", __func__, __LINE__);
            break;
    }

    //free(offset);

    printf ("__%s:%d__\n", __func__, __LINE__);
    g_msg_count++;
}

void ATTime::copyClock(ATTime *src)
{
    printf ("__%s:%d__\n", __func__, __LINE__);
    mLogicalTime = src->mLogicalTime;
    mLogicalCount = src->mLogicalCount;
    mPhysicalTime = src->mPhysicalTime;
    printf ("__%s:%d__\n", __func__, __LINE__);
}

void ATTime::createSendEvent()
{
    printf ("__%s:%d__\n", __func__, __LINE__);
    ATTime *e = &g_attime;
    ATTime *f = new ATTime();

    printf ("__%s:%d__\n", __func__, __LINE__);
    f->mLogicalTime = std::max(e->mLogicalTime, f->mPhysicalTime);
    if (f->mLogicalTime == e->mLogicalTime)
    {
    printf ("__%s:%d__\n", __func__, __LINE__);
        f->mLogicalCount = e->mLogicalCount + 1;
    }
    else
    {
    printf ("__%s:%d__\n", __func__, __LINE__);
        f->mLogicalCount = 0;
    }

    printf ("__%s:%d__\n", __func__, __LINE__);
    g_attime.copyClock(f);
    writeState(g_logfile, 0);
    
    printf ("__%s:%d__\n", __func__, __LINE__);
    delete f;
    printf ("__%s:%d__\n", __func__, __LINE__);
}

void ATTime::createRecvEvent(__uint64_t msgLogicalTime, __uint64_t msgLogicalCount, __uint64_t msgPhysicalTime, char *recvString, ATTime *f)
{
    printf ("__%s:%d__\n", __func__, __LINE__);
    ATTime *e = &g_attime;

    f->mLogicalTime = std::max(e->mLogicalTime, std::max(msgLogicalTime, f->mPhysicalTime));

    printf ("__%s:%d__\n", __func__, __LINE__);
    if ((f->mLogicalTime == e->mLogicalTime) && (f->mLogicalTime == msgLogicalTime))
    {
    printf ("__%s:%d__\n", __func__, __LINE__);
        f->mLogicalCount = std::max(e->mLogicalCount, msgLogicalCount) + 1;
    }
    else if (f->mLogicalTime == e->mLogicalTime)
    {
    printf ("__%s:%d__\n", __func__, __LINE__);
        f->mLogicalCount = e->mLogicalCount + 1;
    }
    else if (f->mLogicalTime == msgLogicalTime)
    {
    printf ("__%s:%d__\n", __func__, __LINE__);
        f->mLogicalCount = msgLogicalCount + 1;
    }
    else
    {
    printf ("__%s:%d__\n", __func__, __LINE__);
        f->mLogicalCount = 0;
    }

    printf ("__%s:%d__\n", __func__, __LINE__);
    g_attime.copyClock(f);
    writeState(g_logfile, 1, recvString);

    printf ("__%s:%d__\n", __func__, __LINE__);
    delete f;
}

char* GetOffset()
{
	FILE *fp;
  int status;
  char path[1035];
    printf ("__%s:%d__\n", __func__, __LINE__);
	char *ret = (char*)malloc(40);

    printf ("__%s:%d__\n", __func__, __LINE__);
  /* Open the command for reading. */
  fp = popen("ntpdc -cloopinfo | grep offset", "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    printf ("__%s:%d__\n", __func__, __LINE__);
    exit(1);
  }

    printf ("__%s:%d__\n", __func__, __LINE__);
  FILE* fp1 = popen("ntpdc -ckerninfo | grep offset", "r");
  if (fp1 == NULL) {
    printf ("__%s:%d__\n", __func__, __LINE__);
    printf("Failed to run command\n" );
    exit(1);
  }
  
    printf ("__%s:%d__\n", __func__, __LINE__);
  char path1[1035];
  fgets(path1, sizeof(path)-1, fp1);
    printf ("__%s:%d__\n", __func__, __LINE__);
 strtok(path1, ":");
        char *plloffsets = strtok(NULL, ":");
        char *plloffset = strtok(plloffsets, " ");
  /* Read the output a line at a time - output it. */
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    printf ("__%s:%d__\n", __func__, __LINE__);
	strtok(path, ":");    
	char *offsets = strtok(NULL, ":");
	char *offset = strtok(offsets, " ");
        sprintf(ret, "%s|%s", offset,plloffset);
    printf ("__%s:%d__\n", __func__, __LINE__);
  }

    printf ("__%s:%d__\n", __func__, __LINE__);
  /* close */
  pclose(fp);
  pclose(fp1);
    printf ("__%s:%d__\n", __func__, __LINE__);
	return ret;
}

void* Receiver(void* dummy)
{
    fd_set rfds;
    int i, err, bytesRecvd, bytesRem;

    printf ("__%s:%d__\n", __func__, __LINE__);
    char buffer [BUFSIZE];
    char buffercopy[BUFSIZE];
    char *bufferHead = NULL;
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;

    printf ("__%s:%d__\n", __func__, __LINE__);
    init((char **)dummy);

    printf ("__%s:%d__\n", __func__, __LINE__);
    while (1)
    {
        FD_ZERO(&rfds);

    printf ("__%s:%d__\n", __func__, __LINE__);
        for(i=0; i < g_peerCount; i++)
        {
    printf ("__%s:%d__\n", __func__, __LINE__);
            if (g_peerFds[i] != -1)
                FD_SET(g_peerFds[i], &rfds);
        }
        err = select(g_maxFd + 1, &rfds, NULL, NULL, &timeout);

    printf ("__%s:%d__\n", __func__, __LINE__);
        if (err == -1)
        {
    printf ("__%s:%d__\n", __func__, __LINE__);
            dieWithMessage("select() failed");
        }
        else if (err)
        {
    printf ("__%s:%d__\n", __func__, __LINE__);
            for (i = 0; i < g_peerCount; i++)
            {
    printf ("__%s:%d__\n", __func__, __LINE__);
                if (g_peerFds[i] == -1) // this peer already exited
                    continue;

                if(FD_ISSET(g_peerFds[i], &rfds))
                {
                    bufferHead = buffer;
    printf ("__%s:%d__\n", __func__, __LINE__);
                    bytesRem = BUFSIZE;
                    while(bytesRem)
                    {
    printf ("__%s:%d__\n", __func__, __LINE__);
                        bytesRecvd = recv(g_peerFds[i], bufferHead, BUFSIZE, 0);
                        if(bytesRecvd < 0)
                        {
    printf ("__%s:%d__\n", __func__, __LINE__);
                            if (errno == EINTR || errno == EAGAIN)
                            {
    printf ("__%s:%d__\n", __func__, __LINE__);
                                printf ("recv() failed with error %d. Retrying...\n", errno);
                                continue;
                            }
                            else
                            {
    printf ("__%s:%d__\n", __func__, __LINE__);
                                close(g_peerFds[i]);
                                g_peerFds[i] = -1;
                                break;
                            }
                        }
                        else if(bytesRecvd == 0)
                        {
    printf ("__%s:%d__\n", __func__, __LINE__);
                            close(g_peerFds[i]);
                            g_peerFds[i] = -1;
                            break;
                        }

                        bytesRem -= bytesRecvd;
                        bufferHead += bytesRecvd;
                    }

                    if (bytesRem != 0)
                    {
                        continue; // something wrong ignore this message
                    }

                    ATTime *f = new ATTime();
                    strcpy(buffercopy, buffer);
                    char * chClient = strtok(buffer, ":");
                    char * strLogClk = strtok(NULL,":");
                    char * strLogCnt = strtok(NULL,":");
                    char * strPhyTime = strtok(NULL,":");

    printf ("__%s:%d__\n", __func__, __LINE__);
    if (strLogClk == NULL)
    {
    printf ("__%s:%d__\n", __func__, __LINE__);
    }
    printf ("%s\n", strLogClk);
    printf ("%s\n", strLogCnt);
    printf ("%s\n", strPhyTime);
                    __uint64_t LogClk = strtol(strLogClk,NULL,10);
                    __uint64_t LogCnt = strtol(strLogCnt,NULL,10);
                    __uint64_t PhyTime = strtol(strPhyTime,NULL,10);

    printf ("__%s:%d__\n", __func__, __LINE__);
                    pthread_mutex_lock(&g_lock_lc);
                    g_attime.createRecvEvent(LogClk, LogCnt, PhyTime, buffercopy, f);
                    pthread_mutex_unlock(&g_lock_lc);
                }
            }
        }
        else
        {
    printf ("__%s:%d__\n", __func__, __LINE__);
            for(i=0; i < g_peerCount; i++)
            {
    printf ("__%s:%d__\n", __func__, __LINE__);
                if (g_peerFds[i] != -1)
                {
    printf ("__%s:%d__\n", __func__, __LINE__);
                    close(g_peerFds[i]);
                    g_peerFds[i] = -1;
                }
            }
    printf ("__%s:%d__\n", __func__, __LINE__);
            return NULL; // timeout of 30 seconds. all done!
        }
    }
}

void init (char* argv[])
{
    int i, err, numBytes, retryCount;
    in_port_t remotePort;
    struct sockaddr_in remoteAddr;
    char *strIp, *strPort;

    printf ("__%s:%d__\n", __func__, __LINE__);
    char buffer[BUFSIZE];

    if (argv == NULL)
        return;

    printf ("__%s:%d__\n", __func__, __LINE__);
    g_peerFds = (int *)malloc(g_peerCount * sizeof(int));
    if (g_peerFds == NULL)
        dieWithMessage("memory too low");

    printf ("__%s:%d__\n", __func__, __LINE__);
    g_maxFd = 0;

    printf ("__%s:%d__\n", __func__, __LINE__);
    for (i = 0; i < g_peerCount; i++)
    {
    printf ("__%s:%d__\n", __func__, __LINE__);
        retryCount = 100;

    printf ("__%s:%d__\n", __func__, __LINE__);
        strIp = strtok(argv[i], ":");
        strPort = strtok(NULL, ":");

        remotePort = atoi(strPort);

        memset(&remoteAddr, 0, sizeof(remoteAddr));
        remoteAddr.sin_family = AF_INET;

    printf ("__%s:%d__\n", __func__, __LINE__);
        err = inet_pton(AF_INET, strIp, &remoteAddr.sin_addr.s_addr);
        if(err == 0)
            dieWithMessage("inet_pton() failed - Invalid address string");
        else if(err < 0)
            dieWithMessage("inet_pton() failed");
        remoteAddr.sin_port = htons(remotePort);

    printf ("__%s:%d__\n", __func__, __LINE__);
        g_peerFds[i] = socket(AF_INET, SOCK_STREAM, 0);
        while((connect(g_peerFds[i], (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) < 0) && (retryCount > 0))
        {
    printf ("__%s:%d__\n", __func__, __LINE__);
            sleep(2);
            retryCount--;
        }

    printf ("__%s:%d__\n", __func__, __LINE__);
        if (retryCount == 0)
            dieWithMessage("Connect failed");
        printf("Connected to %s\n", strIp);
        g_maxFd = (g_peerFds[i] > g_maxFd) ? g_peerFds[i] : g_maxFd;
    printf ("__%s:%d__\n", __func__, __LINE__);
    }
}

int main (int argc, char* argv[])
{
    char *filename = (char *)"dump.log";

    printf ("__%s:%d__\n", __func__, __LINE__);
    if(argc < 3)
    {    
        printf("\nUSAGE: peer myID <peer1> [<peer2> .......]\n");
        exit(1);
    }

    g_buffer = (char *)malloc(BUFSIZE * NUM_MESSAGES * (argc - 1));
    if (g_buffer == NULL)
    {
        printf("Memory too low\n");
        exit(1);
    }

    printf ("__%s:%d__\n", __func__, __LINE__);
    g_logfile = fopen("events.log", "w");
    assert (g_logfile != NULL);

    //set current peer's ID in myID
    sprintf(g_myID, "%s", argv[1]);

    //initialize the logical clock mutex
    if (pthread_mutex_init(&g_lock_lc, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    printf ("__%s:%d__\n", __func__, __LINE__);
    g_peerCount = argc - 2;

    printf ("__%s:%d__\n", __func__, __LINE__);
    //spawn the receiver
    int err = pthread_create(&thread_id, NULL, &Receiver, (void*)&argv[2]);
    if (err != 0)
    {
    printf ("__%s:%d__\n", __func__, __LINE__);
        printf("\ncan't create thread :[%s]", strerror(err));
        return 1;
    }

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

    printf ("__%s:%d__\n", __func__, __LINE__);
    // Bind to the local address
    while(bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
    {
    printf ("__%s:%d__\n", __func__, __LINE__);
        printf("bind() failed");
        sleep(10);
    }

    // Mark the socket so it will listen for incoming connections
    if(listen(servSock, g_peerCount) < 0)
        dieWithMessage("listen() failed");

    printf ("__%s:%d__\n", __func__, __LINE__);
    for (int i=0; i < g_peerCount; i++)
    {
    printf ("__%s:%d__\n", __func__, __LINE__);
        struct sockaddr_in clntAddr; // Client address
        // Set length of client address structure (in-out parameter)
        socklen_t clntAddrLen = sizeof(clntAddr);
        // Wait for a client to connect
        sendFds[i] = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
        if (sendFds[i] < 0)
            dieWithMessage("accept() failed");
    printf ("__%s:%d__\n", __func__, __LINE__);
    }

    printf ("Accepted connections from all peers\n");

    //Send Logic starts
    char message[BUFSIZE];
    char *messageHead = NULL;
    int sleepTime = 0;
    int bytesRem;
    int bytesSent;

    char *offset = (char*)"";
    for (int k = 0; k < NUM_MESSAGES; k++)
    {
    printf ("__%s:%d__\n", __func__, __LINE__);
        for (int i = 0; i < g_peerCount; i++)
        {
    printf ("__%s:%d__\n", __func__, __LINE__);
            if (sendFds[i] == -1)
                break;

            //char *offset = GetOffset();
            pthread_mutex_lock(&g_lock_lc);
            g_attime.createSendEvent();
            g_attime.mPhysicalTime = getCurrentPhysicalTime();
            sprintf(message, "%s:%ld:%ld:%ld:%s", g_myID, g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime, offset);
            //sprintf(message, "%s:%ld:%ld:%ld", g_myID, g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime);
            pthread_mutex_unlock(&g_lock_lc);

            bytesRem = BUFSIZE;
            messageHead = message;
            while (bytesRem)
            {
    printf ("__%s:%d__\n", __func__, __LINE__);
                bytesSent = send(sendFds[i], messageHead, BUFSIZE, 0);
                if (bytesSent == -1 && errno == ECONNRESET)
                {
                    close(sendFds[i]);
                    sendFds[i] = -1;
    printf ("__%s:%d__\n", __func__, __LINE__);
                    break;
                }
                bytesRem -= bytesSent;
                messageHead += bytesSent;
            }
            
            //free(offset);
        }
        //usleep(250000);
        //sleepTime = rand() % 5;
        //sleep(sleepTime);
    }

    for (int i = 0; i < g_peerCount; i++)
    {
        if (sendFds[i] != -1)
        {
    printf ("__%s:%d__\n", __func__, __LINE__);
            close(sendFds[i]);
            sendFds[i] = -1;
        }
    }

    sleep(30);
    printf ("__%s:%d__\n", __func__, __LINE__);
    dumpBufferToFile(g_logfile);
    printf ("__%s:%d__\n", __func__, __LINE__);
    fclose(g_logfile);
    printf ("__%s:%d__\n", __func__, __LINE__);
    return 0;
}
