// includes
#include "event.h"

// globals
ATStack *eventStack = NULL;

ATReturn initATEvent ()
{
	if (eventStack != NULL)
		return AT_ALREADY_INITIALIZED;

	createATStack (&eventStack);
	if (eventStack == NULL)
		return AT_FAIL;

	return AT_SUCCESS;
}

ATReturn uninitATEvent ()
{
	if (eventStack == NULL)
		return AT_UN_INITIALIZED;

	freeATStack (eventStack);
	eventStack = NULL;

	return AT_SUCCESS;
}

ATReturn createEvent (ATEvent **ppEvent)
{
	ATEvent *atEvent = NULL;

	atEvent = (ATEvent*)malloc(sizeof(ATEvent));
	if (atEvent == NULL)
		return AT_LOW_MEMORY;

	if (createATTime(&(atEvent->atTime)) != AT_SUCCESS)
	{
		freeATTime (atEvent->atTime);
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

	return retVal;
}

ATReturn createSendEvent (ATEvent **ppEvent)
{
	ATEvent *sendEvent = NULL;
	at_time eventLogicalTime, eventLogicalCount;
	ATTime *currentTime = NULL;

	if (ppEvent == NULL)
		return AT_NULL_PARAM;

	if (createATTime(&currentTime) != AT_SUCCESS)
		return AT_FAIL;

	if (createEvent(&sendEvent) != AT_SUCCESS)
	{
		freeATTime (currentTime);
		return AT_FAIL;
	}

	SET_LC_TIME (currentTime->lc, getLCTime())
	SET_LC_COUNT (currentTime->lc, getLCCount())
	SET_PC_TIME (currentTime->pc, getPCTime())

	eventLogicalTime = AT_MAX(GET_LC_TIME(currentTime->lc), GET_PC_TIME(currentTime->pc));

	if (eventLogicalTime == GET_LC_TIME(currentTime->lc))
		eventLogicalCount = GET_LC_COUNT(currentTime->lc) + 1;
	else
		eventLogicalCount = 0;

	sendEvent->eventType = AT_SEND_EVENT;
	SET_LC_TIME (sendEvent->atTime->lc, eventLogicalTime)
	SET_LC_COUNT (sendEvent->atTime->lc, eventLogicalCount)
	SET_PC_TIME (sendEvent->atTime->pc, GET_PC_TIME(currentTime->pc))

	ATStackPush (eventStack, sendEvent);

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

	if (ppEvent == NULL || messageTime == NULL)
		return AT_NULL_PARAM;

	if (messageTime->lc == NULL || messageTime->pc == NULL)
		return AT_FAIL;

	if (createATTime(&lastEventTime) != AT_SUCCESS)
		return AT_FAIL;

	if (createATTime(&currentTime) != AT_SUCCESS)
	{
		freeATTime (lastEventTime);
		return AT_FAIL;
	}

	lastEvent = (ATEvent*)ATStackTop(eventStack);
	if (lastEvent != NULL)
	{
		AT_COPY_TIME(lastEventTime, lastEvent->atTime)
	}
	else
	{
		AT_TIME_ZERO (lastEventTime)
	}

	if (createEvent(&recvEvent) != AT_SUCCESS)
	{
		freeATTime(currentTime);
		freeATTime(lastEventTime);

		return AT_FAIL;
	}

	SET_LC_TIME (currentTime->lc, getLCTime())
	SET_LC_COUNT (currentTime->lc, getLCCount())
	SET_PC_TIME (currentTime->pc, getPCTime())

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

		recvEvent->eventType = AT_RECT_EVENT;
		SET_LC_TIME (recvEvent->atTime->lc, eventLogicalTime)
		SET_LC_COUNT (recvEvent->atTime->lc, eventLogicalCount)
		SET_PC_TIME (recvEvent->atTime->pc, GET_PC_TIME(currentTime->pc))

		setLCTime (eventLogicalTime);
		setLCCount (eventLogicalCount);
	}

	*ppEvent = recvEvent;
	freeATTime (lastEventTime);
	freeATTime (currentTime);

	return AT_SUCCESS;
}