// includes
#include "common.h"
#include "event.h"
#include "clock.h"

#include <zmq.h>
#include <pthread.h>

void* startReceiver (void *data)
{
    void *context = NULL, *receiver = NULL;
    char buffer [24];
    int i, rc;

    ATEvent *newEvent = NULL;
    ATTime *messageTime = NULL;
    createATTime (&messageTime);

    context = zmq_ctx_new ();
    receiver = zmq_socket (context, ZMQ_REP);
    rc = zmq_bind (receiver, "tcp://*:12345");
    assert (rc == 0);

    i = 0;
    while (i < 2000)
    {
        if (zmq_recv (receiver, buffer, 24, ZMQ_NOBLOCK) != 0)
        {
            sleep (1);
            continue;
        }
        SET_LC_TIME (messageTime->lc, *((at_time*)(&buffer[0])))
        SET_LC_COUNT (messageTime->lc, *((at_time*)(&buffer[8])))
        SET_PC_TIME (messageTime->lc, *((at_time*)(&buffer[16])))
        createRecvEvent (&newEvent, messageTime);

        i++;
    }

    freeATTime (messageTime);

    return NULL;
}

void* startSender (void *data)
{
    char buffer[24];
    char receiverUrl[24];
    int i, rc;
    void *context = NULL, *sender = NULL;
    char *receiver = (char*)data;

    ATEvent *newEvent = NULL;
    ATTime *messageTime = NULL;
    createATTime (&messageTime);

    if (receiver == NULL)
        return NULL;
    
    context = zmq_ctx_new ();
    sender = zmq_socket (context, ZMQ_REQ);

    sprintf(receiverUrl, "tcp://%s:12345", receiver);
    rc = zmq_connect (sender, receiverUrl);
    assert (rc == 0);

    i = 0;
    while (i < 1000)
    {
        getATTime (messageTime);
        sprintf(buffer, "%llu%llu%llu", GET_LC_TIME(messageTime->lc), GET_LC_COUNT(messageTime->lc), GET_PC_TIME(messageTime->pc));
        if (zmq_send(sender, buffer, 24, ZMQ_NOBLOCK) != 0)
        {
            sleep (1);
            continue;
        }
        createSendEvent (&newEvent);

        i++;
    }

    freeATTime (messageTime);

    return NULL;
}

int main (int argc, char** argv)
{
    int err;
    pthread_t sender1, sender2, receiver;

    if (argc != 3)
        return -1;

    // initializations
    if (initATClock() != AT_SUCCESS)
        return -1;
    if (initATEvent() != AT_SUCCESS)
        return -1;

    err = pthread_create(&receiver, NULL, &startReceiver, (void*)NULL);
    assert (err == 0);
    sleep (10);
    err = pthread_create(&sender1, NULL, &startSender, (void*)argv[1]);
    assert (err == 0);
    err = pthread_create(&sender2, NULL, &startSender, (void*)argv[2]);
    assert (err == 0);

    dumpEvents ("dump.log");

    // uninitializations
    uninitATEvent();
    uninitATClock();

    return 0;
}
