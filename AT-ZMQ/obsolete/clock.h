#ifndef __CLOCK_H__
#define __CLOCK_H__

// includes
#include "common.h"

// typedefs
typedef uint64_t at_time;

typedef struct logicalTime
{
    at_time time;
    at_time count;            // TODO: use "x" last bits of time for count
}LogicalTime;

typedef struct physicalTime
{
    at_time time;
}PhysicalTime;

typedef struct atTime
{
    LogicalTime *lc;
    PhysicalTime *pc;
}ATTime;

// defines
#define GET_LC_TIME(AT_LC) (((AT_LC) != NULL) ? (AT_LC)->time : 0)
#define GET_LC_COUNT(AT_LC) (((AT_LC) != NULL) ? (AT_LC)->count : 0)
#define GET_PC_TIME(AT_PC) (((AT_PC) != NULL) ? (AT_PC)->time : 0)

#define SET_LC_TIME(AT_LC, AT_TIME) { \
    if ((AT_LC) != NULL) \
    { \
        (AT_LC)->time = (AT_TIME); \
    } \
}

#define SET_LC_COUNT(AT_LC, AT_COUNT) { \
    if ((AT_LC) != NULL) \
    { \
        (AT_LC)->count = (AT_COUNT); \
    } \
}

#define SET_PC_TIME(AT_PC, AT_TIME) { \
    if ((AT_PC) != NULL) \
    { \
        (AT_PC)->time = (AT_TIME); \
    } \
}

#define AT_COPY_TIME(AT_DESTINATION, AT_SOURCE) { \
    if ((AT_DESTINATION) != NULL && (AT_SOURCE) != NULL) \
    { \
        if ((AT_DESTINATION)->lc != NULL && (AT_DESTINATION)->pc != NULL && (AT_SOURCE)->lc != NULL && (AT_SOURCE)->pc != NULL) \
        { \
            (AT_DESTINATION)->lc->time = (AT_SOURCE)->lc->time; \
            (AT_DESTINATION)->lc->count = (AT_SOURCE)->lc->count; \
            (AT_DESTINATION)->pc->time = (AT_SOURCE)->pc->time; \
        } \
    } \
}

#define AT_TIME_ZERO(AT_TIME) { \
    if ((AT_TIME) != NULL) \
    { \
        if ((AT_TIME)->lc != NULL && (AT_TIME)->pc != NULL) \
        { \
            (AT_TIME)->lc->time = 0; \
            (AT_TIME)->lc->count = 0; \
            (AT_TIME)->pc->time = 0; \
        } \
    } \
}

// declarations
ATReturn getLCTime (at_time *time);
ATReturn getLCCount (at_time *count);
ATReturn getPCTime (at_time *time);
ATReturn getATTime (ATTime *time);
ATReturn setLCTime (at_time time);
ATReturn setLCCount (at_time count);
ATReturn setPCTime (at_time time);
ATReturn resetPC ();
ATReturn resetLC ();
ATReturn createATTime (ATTime **ppATTime);
ATReturn freeATTime (ATTime *pTime);
ATReturn initATClock ();
ATReturn uninitATClock ();

#endif /* __CLOCK_H__ */
