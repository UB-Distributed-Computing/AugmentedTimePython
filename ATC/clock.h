#ifndef __CLOCK_H__
#define __CLOCK_H__

// includes
#include "common.h"

// typedefs
typedef int at_time;

typedef struct logicalClock
{
	at_time time;
	at_time count;			// TODO: use "x" last bits of time for count
}LogicalClock;

typedef struct physicalClock
{
	at_time time;
}PhysicalClock;

typedef struct atClock
{
	LogicalClock *lc;
	PhysicalClock *pc;
}ATClock;

// definitions
at_time getLC ();
at_time getLCCount ();
at_time getPC ();
void setLC (at_time time);
void setLCCount (at_time count);
void setPC (at_time time);
ATReturn resetPC ();
ATReturn resetLC ();
ATReturn initATClock ();
ATReturn uninitATClock ();

#endif __CLOCK_H__