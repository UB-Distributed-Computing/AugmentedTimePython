#ifndef __EVENT_H__
#define __EVENT_H__

// includes
#include "common.h"
#include "clock.h"
#include "stack.h"

// typedefs
typedef enum
{
	AT_LOCAL_EVENT,
	AT_RECT_EVENT,
	AT_SEND_EVENT
}ATEventType;

typedef struct atEvent
{
	ATEventType eventType;
	PhysicalClock pc;
	LogicalClock lc;
}ATEvent;

// declarations
ATReturn initATEvent ();
ATReturn uninitATEvent ();
ATEvent* createEvent ();
void freeEvent (ATEvent *event);
ATEvent* createSendEvent ();
ATEvent* createRecvEvent (at_time messagePC, at_time messageLC, at_time messageLCCount);

#endif __EVENT_H__