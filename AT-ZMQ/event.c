// includes
#include "event.h"

// globals
ATStack *eventStack = NULL;
FILE *logFile = NULL;

ATReturn writeEvent (void *data, FILE *f)
{
    ATEvent *atEvent = NULL;

    if (data == NULL || f == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NULL_PARAM;
    }

    atEvent = (ATEvent*)(data);
    fprintf (f, "\n");
    switch (atEvent->eventType)
    {
        case AT_RECV_EVENT:
            fprintf (f, "Event Type: Receive\n");
            break;
        case AT_SEND_EVENT:
            fprintf (f, "Event Type: Send\n");
            break;
        case AT_LOCAL_EVENT:
            fprintf (f, "Event Type: Local\n");
            break;
        default:
            AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
            fprintf (f, "Event Type: Unknown\n");
            break;
    }
    fprintf (f, "Time: logical = %llu, count = %llu, physical = %llu\n",(long long unsigned) GET_LC_TIME(atEvent->atTime->lc),
            (long long unsigned)GET_LC_COUNT(atEvent->atTime->lc),(long long unsigned)GET_PC_TIME(atEvent->atTime->pc));
    fprintf (f, "\n");

    freeEvent (atEvent);

    return AT_SUCCESS;
}

ATReturn initATEvent (char *logFileName)
{
    if (eventStack != NULL || logFile != NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_ALREADY_INITIALIZED;
    }

    if (logFileName == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NULL_PARAM;
    }

    createATStack (&eventStack);
    if (eventStack == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    logFile = fopen(logFileName, "a");
    if (logFile == NULL)
    {
        freeATStack (eventStack);
        eventStack = NULL;

        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    return AT_SUCCESS;
}

ATReturn uninitATEvent ()
{
    ATEvent *atEvent = NULL;

    if (eventStack == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NOT_INITIALIZED;
    }

    while (1)
    {
        if (ATStackPop((void **)(&atEvent), eventStack) != AT_SUCCESS)
            break;
        if (atEvent == NULL)
            break;

        freeEvent (atEvent);
    }

    freeATStack (eventStack);
    eventStack = NULL;

    if (logFile != NULL)
    {
        fclose(logFile);
        logFile = NULL;
    }

    return AT_SUCCESS;
}

ATReturn createEvent (ATEvent **ppEvent)
{
    ATEvent *atEvent = NULL;

    atEvent = (ATEvent*)malloc(sizeof(ATEvent));
    if (atEvent == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_LOW_MEMORY;
    }

    if (createATTime(&(atEvent->atTime)) != AT_SUCCESS)
    {
        freeATTime (atEvent->atTime);
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    *ppEvent = atEvent;

    return AT_SUCCESS;
}

ATReturn freeEvent (ATEvent *atEvent)
{
    ATReturn retVal = AT_SUCCESS;

    if (atEvent != NULL)
    {
        retVal = freeATTime (atEvent->atTime);
        free (atEvent);
    }

    if (retVal != AT_SUCCESS)
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);

    return retVal;
}

ATReturn createSendEvent (ATEvent **ppEvent)
{
    ATEvent *sendEvent = NULL;
    at_time eventLogicalTime, eventLogicalCount;
    ATTime *currentTime = NULL;

    if (ppEvent == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NULL_PARAM;
    }

    if (createATTime(&currentTime) != AT_SUCCESS)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    if (createEvent(&sendEvent) != AT_SUCCESS)
    {
        freeATTime (currentTime);
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    if (getATTime(currentTime) != AT_SUCCESS)
    {
        freeATTime (currentTime);
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    eventLogicalTime = AT_MAX(GET_LC_TIME(currentTime->lc), GET_PC_TIME(currentTime->pc));

    if (eventLogicalTime == GET_LC_TIME(currentTime->lc))
        eventLogicalCount = GET_LC_COUNT(currentTime->lc) + 1;
    else
        eventLogicalCount = 0;

    sendEvent->eventType = AT_SEND_EVENT;
    SET_LC_TIME (sendEvent->atTime->lc, eventLogicalTime)
    SET_LC_COUNT (sendEvent->atTime->lc, eventLogicalCount)
    SET_PC_TIME (sendEvent->atTime->pc, GET_PC_TIME(currentTime->pc))

    ATStackPush (eventStack, sendEvent, logFile, &writeEvent);

    setLCTime (eventLogicalCount);
    setLCCount (eventLogicalCount);

    *ppEvent = sendEvent;
    freeATTime (currentTime);

    return AT_SUCCESS;
}

ATReturn createRecvEvent (ATEvent **ppEvent, ATTime *messageTime)
{
    ATEvent *recvEvent = NULL, *lastEvent = NULL;
    at_time eventLogicalTime, eventLogicalCount;
    ATTime *currentTime = NULL, *lastEventTime = NULL;
    void *top = NULL;

    if (ppEvent == NULL || messageTime == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NULL_PARAM;
    }

    if (messageTime->lc == NULL || messageTime->pc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    if (createATTime(&lastEventTime) != AT_SUCCESS)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    if (createATTime(&currentTime) != AT_SUCCESS)
    {
        freeATTime (lastEventTime);
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    if (ATStackTop(&top, eventStack) != AT_SUCCESS)
    {
        freeATTime (lastEventTime);
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    lastEvent = (ATEvent*)top;
    if (lastEvent != NULL)
    {
        AT_COPY_TIME(lastEventTime, lastEvent->atTime)
    }
    else
    {
        AT_TIME_ZERO (lastEventTime)
    }

    if (getATTime(currentTime) != AT_SUCCESS)
    {
        freeATTime(currentTime);
        freeATTime(lastEventTime);

        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    if (createEvent(&recvEvent) != AT_SUCCESS)
    {
        freeATTime(currentTime);
        freeATTime(lastEventTime);

        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    if (recvEvent != NULL)
    {
        eventLogicalTime = AT_MAX (GET_LC_TIME(lastEventTime->lc), GET_LC_TIME(messageTime->lc));
        eventLogicalTime = AT_MAX (eventLogicalTime, GET_PC_TIME(currentTime->pc));

        if ((eventLogicalTime == GET_LC_TIME(lastEventTime->lc)) && (eventLogicalTime == GET_LC_TIME(messageTime->lc)))
        {
            eventLogicalCount = AT_MAX (GET_LC_COUNT(lastEventTime->lc), GET_LC_COUNT(messageTime->lc)) + 1;
        }
        else if (eventLogicalTime == GET_LC_TIME(lastEventTime->lc))
        {
            eventLogicalCount = GET_LC_COUNT(lastEventTime->lc) + 1;
        }
        else if (eventLogicalTime == GET_LC_TIME(messageTime->lc))
        {
            eventLogicalCount = GET_LC_COUNT(messageTime->lc) + 1;
        }
        else
        {
            eventLogicalCount = 0;
        }

        recvEvent->eventType = AT_RECV_EVENT;
        SET_LC_TIME (recvEvent->atTime->lc, eventLogicalTime)
        SET_LC_COUNT (recvEvent->atTime->lc, eventLogicalCount)
        SET_PC_TIME (recvEvent->atTime->pc, GET_PC_TIME(currentTime->pc))

        setLCTime (eventLogicalTime);
        setLCCount (eventLogicalCount);
    }

    ATStackPush (eventStack, recvEvent, logFile, &writeEvent);

    *ppEvent = recvEvent;
    freeATTime (lastEventTime);
    freeATTime (currentTime);

    return AT_SUCCESS;
}
