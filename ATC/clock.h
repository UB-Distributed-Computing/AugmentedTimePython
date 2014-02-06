#ifndef __CLOCK_H__
#define __CLOCK_H__

// includes
#include "common.h"

// typedefs
typedef unint64_t at_time;

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

#endif __CLOCK_H__