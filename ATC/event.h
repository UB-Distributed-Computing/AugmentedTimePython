#ifndef __EVENT_H__
#define __EVENT_H__

// includes
#include "common.h"

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
}ATEvent;

#endif __EVENT_H__