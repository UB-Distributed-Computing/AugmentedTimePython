#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>

int MAX_CLIENTS = 10;
int DELTA_FREQ_SEC = 20;
pthread_mutex_t offset_lock;

pthread_t thread_id;
double nOffsets[10];
char *outputFile = "Deltas";

int Length(char *string)
{
	int len = 0;
	while(string[len] !='\0')
		len++;
	return len;
}

void WriteToFile(char *file, float delta)
{
	FILE *fp;
	char buf[20];
	sprintf(buf,"%f\n", delta);
	fp=fopen("file", "a");
	if(fp)
	{
		fwrite(buf, sizeof(buf), Length(buf), fp);
		printf("delta =%s\n", buf);
		fclose(fp);
	}
}

void* FindDelta(void* dummy)
{
	int i;
	double max = 0, min = 0;
	while(1)
	{
		sleep(DELTA_FREQ_SEC);
	
		pthread_mutex_lock(&offset_lock);
		for(i=0; i< MAX_CLIENTS; i++)
		{
			if(nOffsets[i] != 0 && (nOffsets[i] > max || max == 0) )
				max = nOffsets[i];
			if(nOffsets[i] != 0 && (nOffsets[i] < min || min == 0) )
				min = nOffsets[i];
			nOffsets[i] = 0;
		}
		printf("\nmax = %f, min = %f\n", max, min);
		pthread_mutex_unlock(&offset_lock);
		if(max != 0 && min != 0)
		{ 
			double delta = max - min;
			WriteToFile(outputFile, delta);
		}
	}
}	

int main (void)
{
// Socket to talk to clients
	void *context = zmq_ctx_new ();
	void *responder = zmq_socket (context, ZMQ_REP);
	int rc = zmq_bind (responder, "tcp://*:12345");
	printf("%d", rc);
	assert (rc == 0);
	if (pthread_mutex_init(&offset_lock, NULL) != 0)
	{
		printf("\n mutex init failed\n");
		return 1;
	}

	char buffer [10];
	int count = 5;
	char list[10];
	int i;
	
	for(i=0; i<MAX_CLIENTS; i++)
		nOffsets[i] = 0;
	
	//Start the thread to find delta;
	int err = pthread_create(&thread_id, NULL, &FindDelta, (void*)NULL);
	if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
	

	while (1) 
	{
		char buffer [20];
		zmq_recv (responder, buffer, 20, 0);
			
		char * chClient = strtok(buffer, ":");
		char * strOffset = strtok(NULL,":");
		
		i = atoi (chClient);
		double offset = atof(strOffset);
		pthread_mutex_lock(&offset_lock);
		nOffsets[i] = offset;
		printf("\nrecvd:%f", offset);
		pthread_mutex_unlock(&offset_lock);
		zmq_send (responder, "Worl", 5, 0);
	}
	pthread_mutex_destroy(&offset_lock);
	return 0;
}

