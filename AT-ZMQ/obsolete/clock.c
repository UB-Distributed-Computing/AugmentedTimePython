// includes
#include "clock.h"
#ifndef OS_WIN
#include <sys/time.h>
#else /* OS_WIN */
#include <windows.h>
#endif /* OS_WIN */

// globals
ATTime *atc = NULL;

// definitions
ATReturn getLCTime (at_time *time)
{
    if (time == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NULL_PARAM;
    }

    if (atc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NOT_INITIALIZED;
    }

    if (atc->lc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    *time = atc->lc->time;

    return AT_SUCCESS;
}

ATReturn getLCCount (at_time *count)
{
    if (count == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NULL_PARAM;
    }

    if (atc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NOT_INITIALIZED;
    }

    if (atc->lc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    *count = atc->lc->count;

    return AT_SUCCESS;
}

ATReturn getPCTime (at_time *time)
{
#ifndef OS_WIN
    struct timeval tv;
#endif /* OS_WIN */

    if (time == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NULL_PARAM;
    }

    if (atc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NOT_INITIALIZED;
    }

    if (atc->pc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

#ifndef OS_WIN
    if (gettimeofday(&tv, NULL) != 0)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    atc->pc->time = tv.tv_sec * 1000000 + tv.tv_usec;
#else /* OS_WIN */
    atc->pc->time = GetTickCount64();
#endif /* OS_WIN */

    *time = atc->pc->time;

    return AT_SUCCESS;
}

ATReturn getATTime (ATTime *time)
{
    ATReturn retVal = AT_SUCCESS;

    if (time == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NULL_PARAM;
    }
    if (time->lc == NULL || time->pc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    retVal = getLCTime (&(time->lc->time));
    if (retVal != AT_SUCCESS)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return retVal;
    }

    retVal = getLCCount (&(time->lc->count));
    if (retVal != AT_SUCCESS)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return retVal;
    }

    retVal = getPCTime (&(time->pc->time));
    if (retVal != AT_SUCCESS)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return retVal;
    }

    return AT_SUCCESS;
}

ATReturn setLCTime (at_time time)
{
    if (atc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NOT_INITIALIZED;
    }

    if (atc->lc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    atc->lc->time = time;

    return AT_SUCCESS;
}

ATReturn setLCCount (at_time count)
{
    if (atc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NOT_INITIALIZED;
    }

    if (atc->lc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    atc->lc->count = count;

    return AT_SUCCESS;
}

ATReturn setPCTime (at_time time)
{
    if (atc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NOT_INITIALIZED;
    }

    if (atc->pc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    atc->pc->time = time;

    return AT_SUCCESS;
}

ATReturn resetPC ()
{
#ifndef OS_WIN
    struct timeval tv;
#endif /* OS_WIN */

    if (atc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NOT_INITIALIZED;
    }

    if (atc->pc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

#ifndef OS_WIN
    if (gettimeofday(&tv, NULL) != 0)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    atc->pc->time = tv.tv_sec * 1000000 + tv.tv_usec;
    if (atc->pc->time <= 0)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }
#else /* OS_WIN */
    atc->pc->time = GetTickCount64();
#endif /* OS_WIN */

    return AT_SUCCESS;
}

ATReturn resetLC ()
{
    if (atc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NOT_INITIALIZED;
    }

    if (atc->lc == NULL || atc->pc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    atc->lc->time = atc->pc->time;
    atc->lc->count = 0;

    return AT_SUCCESS;
}

ATReturn createATTime (ATTime **ppATTime)
{
    ATTime *pTime = NULL;

    if (ppATTime == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NULL_PARAM;
    }

    pTime = (ATTime*)malloc(sizeof(ATTime));
    if (pTime == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_LOW_MEMORY;
    }

    pTime->lc = (LogicalTime*)malloc(sizeof(LogicalTime));
    if (pTime->lc == NULL)
    {
        free (pTime);
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_LOW_MEMORY;
    }

    pTime->pc = (PhysicalTime*)malloc(sizeof(PhysicalTime));
    if (pTime->pc == NULL)
    {
        free (pTime->lc);
        free (pTime);

        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_LOW_MEMORY;
    }

    *ppATTime = pTime;

    return AT_SUCCESS;
}

ATReturn freeATTime (ATTime *pTime)
{
    if (pTime == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NULL_PARAM;
    }

    if (pTime->lc == NULL || pTime->pc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_FAIL;
    }

    free (pTime->lc);
    free (pTime->pc);
    free (pTime);

    return AT_SUCCESS;
}

ATReturn initATClock ()
{
    if (atc != NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_ALREADY_INITIALIZED;
    }

    atc = (ATTime*)malloc(sizeof(ATTime));
    if (atc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_LOW_MEMORY;
    }

    atc->lc = (LogicalTime*)malloc(sizeof(LogicalTime));
    if (atc->lc == NULL)
    {
        free (atc);
        atc = NULL;

        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_LOW_MEMORY;
    }

    atc->pc = (PhysicalTime*)malloc(sizeof(PhysicalTime));
    if (atc->pc == NULL)
    {
        free (atc->lc);
        free (atc);
        atc = NULL;

        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_LOW_MEMORY;
    }

    resetPC();
    resetLC();

    return AT_SUCCESS;
}

ATReturn uninitATClock ()
{
    if (atc == NULL)
    {
        AT_LOG ("ERROR: %s:%d\n", __func__, __LINE__);
        return AT_NOT_INITIALIZED;
    }

    if (atc->pc != NULL)
        free (atc->pc);
    if (atc->lc != NULL)
        free (atc->lc);
    free (atc);

    atc = NULL;

    return AT_SUCCESS;
}
