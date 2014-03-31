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
    AT_RECV_EVENT,
    AT_SEND_EVENT
}ATEventType;

typedef struct atEvent
{
    ATEventType eventType;
    ATTime *atTime;
}ATEvent;

// declarations
ATReturn initATEvent (char *logFileName);
ATReturn uninitATEvent ();
ATReturn createEvent (ATEvent **ppEvent);
ATReturn freeEvent (ATEvent *atEvent);
ATReturn createSendEvent (ATEvent **ppEvent);
ATReturn createRecvEvent (ATEvent **ppEvent, ATTime *messageTime);

#endif /* __EVENT_H__ */
