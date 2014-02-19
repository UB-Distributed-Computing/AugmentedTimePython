#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

char* GetOffset()
{
	FILE *fp;
  int status;
  char path[1035];
	char *ret = (char*)malloc(20);

  /* Open the command for reading. */
  fp = popen("ntpdc -cloopinfo | grep offset", "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  /* Read the output a line at a time - output it. */
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
	strtok(path, ":");    
	char *offsets = strtok(NULL, ":");
	char *offset = strtok(offsets, " ");
    sprintf(ret, "%s", offset);
  }

  /* close */
  pclose(fp);
	return ret;
}

int main (int argc, char* argv[])
{
	if(argc != 3)
	{	
		printf("\nUSAGE: client <server> <clientid>\n");
		exit(1);
	}
	char server[20];
	sprintf(server, "tcp://%s", argv[1]);
	void *context = zmq_ctx_new ();
	void *responder = zmq_socket (context, ZMQ_REQ);
	int rc = zmq_connect (responder, server);
	printf("\nServer:%s\n", server);
	assert (rc == 0);

	while(1)
	{
		char* offset = GetOffset();
		char message[20];
		sprintf(message, "%s:%s", argv[2], offset);
		printf("sending: %s\n", message);
		zmq_send(responder, message, 20, 0);
		char buffer[10];
		zmq_recv(responder, buffer,5,0);
		sleep(20);
		free(offset);
	}
	
	return 0;
}

