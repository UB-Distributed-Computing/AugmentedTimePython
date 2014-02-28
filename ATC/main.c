// includes
#include "common.h"
#include "event.h"
#include "clock.h"

int main ()
{
    ATEvent *newEvent = NULL;
    ATTime *currentTime = NULL;

    // initializations
    if (initATClock() != AT_SUCCESS)
        return -1;
    if (initATEvent() != AT_SUCCESS)
        return -1;

    createSendEvent (&newEvent);
    createATTime (&currentTime);
    getATTime (currentTime);
    createRecvEvent (&newEvent, currentTime);

    dumpEvents ("dump.log");

    // uninitializations
    uninitATEvent();
    uninitATClock();

    return 0;
}
