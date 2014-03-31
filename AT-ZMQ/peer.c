#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory.h>
#include <sys/time.h>
#include <algorithm>

char g_myID[3];
pthread_mutex_t g_lock_lc;
unsigned long g_lc;
pthread_t thread_id;

FILE *g_logfile = NULL;

char* GetOffset();

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
            fprintf (fp, "%lu:%lu:%lu:%s\n", g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime, offset);
            break;
        case 1: // recv event
            fprintf (fp, "Recv:");
            fprintf (fp, "%lu:%lu:%lu", g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime);
            fprintf (fp, ":%s:%s\n", recvString, offset);
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

    f->mLogicalTime = std::max(e->mLogicalTime, getCurrentPhysicalTime());
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
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:12345");
    printf("%d", rc);
    assert (rc == 0);

    int client;    
    while (1) 
    {
        char buffer [300];
        buffer[1] = '\0';
        zmq_recv (responder, buffer, 300, 0);
        printf("rcvd:%s\n", buffer);    
        zmq_send (responder, "Worl", 5, 0);            
        fflush(stdout);
        char * chClient = strtok(buffer, ":");
        char * strLogClk = strtok(NULL,":");
        char * strLogCnt = strtok(NULL,":");
        char * strPhyTime = strtok(NULL,":");

        client = atoi (chClient);
        __uint64_t LogClk = strtol(strLogClk,NULL,10);
        __uint64_t LogCnt = strtol(strLogCnt,NULL,10);
        __uint64_t PhyTime = strtol(strPhyTime,NULL,10);

        pthread_mutex_lock(&g_lock_lc);
        g_attime.createRecvEvent(LogClk, LogCnt, PhyTime, buffer);
        pthread_mutex_unlock(&g_lock_lc);
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

    //initialize the logical clock mutex
    if (pthread_mutex_init(&g_lock_lc, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    //spawn the receiver
    int err = pthread_create(&thread_id, NULL, &Receiver, (void*)NULL);
    if (err != 0)
    {
        printf("\ncan't create thread :[%s]", strerror(err));
        return 1;
    }

    int nPeers = argc - 2;
    void **responder = new void*[nPeers];
    void *context = zmq_ctx_new();
    char peerIpPort[20];

    for(int i = 2; i< argc; i++)
    {
        sprintf(peerIpPort, "tcp://%s", argv[i]);
        printf("\nConnecting to %s\n", peerIpPort);
        responder[i-2] = zmq_socket (context, ZMQ_REQ);
        int rc = zmq_connect (responder[i-2], peerIpPort);
        printf("%d\n", rc);
        sleep(2);
        if(rc==0)
        {
            printf("\nConnected to peer:%s\n", peerIpPort);
        }
        assert (rc == 0);
    }
    printf("\nConnected with all the peers\n");

    //Send Logic starts
    char message[300];
    int sleepTime = 0;
    while (1)
    {
        for (int i = 0; i < nPeers; i++)
        {
            pthread_mutex_lock(&g_lock_lc);
            g_attime.createSendEvent();
            char *offset = GetOffset();
            sprintf(message, "%s:%ld:%ld:%ld:%s", g_myID, g_attime.mLogicalTime, g_attime.mLogicalCount, g_attime.mPhysicalTime, offset);
            free(offset);
            pthread_mutex_unlock(&g_lock_lc);

            zmq_send(responder[i], message, 300, 0);
            char buffer[10];
            zmq_recv(responder[i], buffer,5,0);
        }

        //sleepTime = rand() % 5;
        //sleep(sleepTime);
    }

    return 0;
}
