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
		
		//LC logic

		SET_LC_TIME (messageTime->lc, LogClk)
		SET_LC_COUNT (messageTime->lc, LogCnt)
		SET_PC_TIME (messageTime->pc, PhyTime)

		createRecvEvent (&newEvent, messageTime);

		//LC logic ends
//		pthread_mutex_lock(&offset_lock);
//		nOffsets[i] = offset;
//		printf("\nrecvd:%f", offset);
//		pthread_mutex_unlock(&offset_lock);
		
	}

	freeATTime(messageTime);
}

int main (int argc, char* argv[])
{
	ATEvent *newEvent = NULL;
	ATTime *messageTime = NULL;
	createATTime (&messageTime);
	
	if(argc < 3)
	{	
		printf("\nUSAGE: peer myID <peer1> [<peer2> .......]\n");
		exit(1);
	}

    	// initializations
    	if (initATClock() != AT_SUCCESS)
    	    return -1;
    	if (initATEvent() != AT_SUCCESS)
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
           printf("\ncan't create thread :[%s]", strerror(err));
	
	int nPeers = argc - 2;
	//void ** context = new void*[nPeers];
	void ** responder = new void*[nPeers];
	//int * rclist = new int[nPeers]; 
	void *context = zmq_ctx_new ();
	for(int i = 0; i< nPeers; i++)
	{
	}	
	char peerIpPort[20];

	//while(1)//Keep looping till we connect to all the peers. Will have to stick to this unclean approach till we can come up with a better idea
	//{
	//	bool bAllConnected = true;
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
//	sleep(2);
	//	if(bAllConnected)
	//		break;	//no one set it to false means all rc were 0;
	//}
	printf("\nConnected with all the peers\n");

	//Send Logic starts
	char message[300];
	int sleepTime = 0;
	while (1)
	{
		for (int i = 0; i < nPeers; i++)
		{
			getATTime(messageTime);
			long int dummylc = rand() % 10;
			sprintf(message, "%s:%ld:%ld:%ld", g_myID, /*GET_LC_TIME(messageTime->lc)*/ dummylc, GET_LC_COUNT(messageTime->lc), GET_PC_TIME(messageTime->pc));
			printf("sending: %s\n", message);
			zmq_send(responder[i], message, 300, 0);
			char buffer[10];
			zmq_recv(responder[i], buffer,5,0);
			// copy messageTime to send buffer to send

			createSendEvent (&newEvent);
		}

		sleepTime = rand() % 5;
		sleep(sleepTime);

		dumpEvents((char*)"dump.log");
	}

	    // uninitializations
    	uninitATEvent();
    	uninitATClock();


	freeATTime (messageTime);
	//Send logic ends	
return 0;
}