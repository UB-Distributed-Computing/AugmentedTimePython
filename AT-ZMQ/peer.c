#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory.h>
#include "common.h"
#include "event.h"
#include "clock.h"

char g_myID[3];
pthread_mutex_t g_lock_lc;
unsigned long g_lc;
pthread_t thread_id;

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

    ATEvent *newEvent = NULL;
    ATTime *messageTime = NULL;
    createATTime (&messageTime);    

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
        long int LogClk = strtol(strLogClk,NULL,10);
        long int LogCnt = strtol(strLogCnt,NULL,10);
        long int PhyTime = strtol(strPhyTime,NULL,10);
        
        // LC logic
        SET_LC_TIME (messageTime->lc, LogClk)
        SET_LC_COUNT (messageTime->lc, LogCnt)
        SET_PC_TIME (messageTime->pc, PhyTime)

        pthread_mutex_lock(&g_lock_lc);
        createRecvEvent (&newEvent, messageTime);
        pthread_mutex_unlock(&g_lock_lc);
        // LC logic ends
    }

    freeATTime(messageTime);
}

int main (int argc, char* argv[])
{
    ATEvent *newEvent = NULL;
    ATTime *messageTime = NULL;
    char *filename = (char *)"dump.log";

    if(argc < 3)
    {    
        printf("\nUSAGE: peer myID <peer1> [<peer2> .......]\n");
        exit(1);
    }

    // initializations
    if (initATClock() != AT_SUCCESS)
        return -1;
    if (initATEvent(filename) != AT_SUCCESS)
        return -1;

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
            printf("\nConnected to peer:%s\n", peerIpPort);
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

            createSendEvent (&newEvent);
            pthread_mutex_unlock(&g_lock_lc);
            messageTime = newEvent->atTime;
	    char *offset = GetOffset();
            sprintf(message, "%s:%ld:%ld:%ld:%s", g_myID, GET_LC_TIME(messageTime->lc), GET_LC_COUNT(messageTime->lc), GET_PC_TIME(messageTime->pc),offset);
	    free(offset);
            printf("sending: %s\n", message);
            zmq_send(responder[i], message, 300, 0);
            char buffer[10];
            zmq_recv(responder[i], buffer,5,0);
            // copy messageTime to send buffer to send

        }

        sleepTime = rand() % 5;
        sleep(sleepTime);
    }

    // uninitializations
    uninitATEvent();
    uninitATClock();

    //Send logic ends    
    return 0;
}
