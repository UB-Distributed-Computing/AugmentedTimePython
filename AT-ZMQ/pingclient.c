#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <zmq.h>

__uint64_t getCurrentPhysicalTime()
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0)
        assert(!"gettimeofday failed!");

    return tv.tv_sec * 1000000 + tv.tv_usec;
}

int main(int argc, char **argv)
{
    char peerIpPort[100];
    sprintf(peerIpPort, "tcp://%s", argv[1]);
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REQ);
    int rc = zmq_connect (responder, peerIpPort);
    assert (rc == 0);
    int sendTime = getCurrentPhysicalTime();
    zmq_send(responder, "Hello", 300, 0);
    char buffer[10];
    zmq_recv(responder, buffer,5,0);
    printf("delay: %ul",getCurrentPhysicalTime()-sendTime);
}
