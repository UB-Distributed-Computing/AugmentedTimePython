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

void main(int argc, char ** argv)
{
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:12345");
    printf("%d", rc);
    assert (rc == 0);

    while (1) 
    {
        char buffer [300];
        zmq_recv (responder, buffer, 300, 0);
        zmq_send (responder, "World", 6, 0);            
    }
}
