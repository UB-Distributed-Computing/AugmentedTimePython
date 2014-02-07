// includes
#include "event.h"

// globals
ATCStack *eventStack = NULL;

ATReturn initATEvent ()
{
	if (eventStack != NULL)
		return AT_ALREADY_INITIALIZED;

	createATCStack (&eventStack);
	if (eventStack == NULL)
		return AT_FAIL;

	return AT_SUCCESS;
}

ATReturn uninitATEvent ()
{
	if (eventStack == NULL)
		return AT_UN_INITIALIZED;

	freeATCStack (&eventStack);

	return AT_SUCCESS;
}

ATEvent* createEvent ()
{
	ATEvent *event = NULL;

	event = (ATEvent*)malloc(sizeof(ATEvent));

	return event;
}

void freeEvent (ATEvent *event)
{
	if (event != NULL)
		free (event);
}

// To create a send event f at process j with physical clock ph.j
ATEvent* createSendEvent ()
{
	ATEvent *sendEvent = NULL;
	at_time physicalTime, oldLogicalTime, newLogicalTime, count;

	sendEvent = createEvent();
	oldLogicalTime = getLC();
	physicalTime = getPC();
	count = getLCCount();

	if (sendEvent != NULL)
	{
		newLogicalTime = AT_MAX(oldLogicalTime, physicalTime);

		if (newLogicalTime == oldLogicalTime)
			count++;
		else
			count = 0;

		sendEvent->eventType = AT_SEND_EVENT;
		sendEvent->lc.time = newLogicalTime;
		sendEvent->lc.count = count;
		sendEvent->pc.time = physicalTime;

		ATCStackPush (eventStack, sendEvent);

		setLC (newLogicalTime);
		setLCCount (count);
	}

	return sendEvent;
}

ATEvent* createRecvEvent (at_time messagePC, at_time messageLC, at_time messageLCCount)
{
	ATEvent *recvEvent = NULL, *lastEvent = NULL;
	at_time physicalTime, newLogicalTime, count, lastEventPC = 0, lastEventLC = 0, lastEventLCCount = 0;

	lastEvent = (ATEvent*)ATCStackTop(eventStack);
	if (lastEvent != NULL)
	{
		lastEventLC = lastEvent->lc.time;
		lastEventLCCount = lastEvent->lc.count;
		lastEventPC = lastEvent->pc.time;
	}
	recvEvent = createEvent();

	if (recvEvent != NULL)
	{
		physicalTime = getPC();
		newLogicalTime = AT_MAX (lastEventLC, messageLC);
		newLogicalTime = AT_MAX (newLogicalTime, physicalTime);

		if (newLogicalTime == lastEventLC && newLogicalTime == messageLC)
		{
			count = AT_MAX (lastEventLCCount, messageLCCount) + 1;
		}
		else if (newLogicalTime == lastEventLC)
		{
			count = lastEventLCCount + 1;
		}
		else if (newLogicalTime == messageLC)
		{
			count = messageLCCount + 1;
		}
		else
		{
			count = 0;
		}

		recvEvent->eventType = AT_RECT_EVENT;
		recvEvent->lc.time = newLogicalTime;
		recvEvent->lc.count = count;
		recvEvent->pc.time = physicalTime;

		setLC (newLogicalTime);
		setLCCount (count);
	}

	return recvEvent;
}