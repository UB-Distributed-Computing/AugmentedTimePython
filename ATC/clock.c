// includes
#include "clock.h"

// globals
ATTime *atc = NULL;

// definitions
ATReturn getLCTime (at_time *time)
{
	if (time == NULL)
		return AT_NULL_PARAM;

	if (atc == NULL)
		return AT_NOT_INITIALIZED;

	if (atc->lc == NULL)
		return AT_FAIL;

	*time = atc->lc->time;

	return AT_SUCCESS;
}

ATReturn getLCCount (at_time *count)
{
	if (count == NULL)
		return AT_NULL_PARAM;

	if (atc == NULL)
		return AT_NOT_INITIALIZED;

	if (atc->lc == NULL)
		return AT_FAIL;

	*count = atc->lc->count;

	return AT_SUCCESS;
}

ATReturn getPCTime (at_time *time)
{
	if (time == NULL)
		return AT_NULL_PARAM;

	if (atc == NULL)
		return AT_NOT_INITIALIZED;

	if (atc->pc == NULL)
		return AT_FAIL;

	*time = atc->pc->time;

	return AT_SUCCESS;
}

ATReturn setLCTime (at_time time)
{
	if (atc == NULL)
		return AT_NOT_INITIALIZED;

	if (atc->lc == NULL)
		return AT_FAIL;

	atc->lc->time = time;

	return AT_SUCCESS;
}

ATReturn setLCCount (at_time count)
{
	if (atc == NULL)
		return AT_NOT_INITIALIZED;

	if (atc->lc == NULL)
		return AT_FAIL;

	atc->lc->count = count;

	return AT_SUCCESS;
}

ATReturn setPCTime (at_time time)
{
	if (atc == NULL)
		return AT_NOT_INITIALIZED;

	if (atc->pc == NULL)
		return AT_FAIL;

	atc->pc->time = time;

	return AT_SUCCESS;
}

ATReturn resetPC ()
{
	if (atc == NULL)
		return AT_NOT_INITIALIZED;

	if (atc->pc == NULL)
		return AT_FAIL;
	
	// TODO: resetting time means querying time from NTP?
	atc->pc->time = 0;

	return AT_SUCCESS;
}

ATReturn resetLC ()
{
	if (atc == NULL)
		return AT_NOT_INITIALIZED;

	if (atc->lc == NULL || atc->pc == NULL)
		return AT_FAIL;

	atc->lc->time = atc->pc->time;
	atc->lc->count = 0;

	return AT_SUCCESS;
}

ATReturn initATClock ()
{
	if (atc != NULL)
		return AT_ALREADY_INITIALIZED;

	atc = (ATTime*)malloc(sizeof(ATTime));
	if (atc == NULL)
		return AT_LOW_MEMORY;

	atc->lc = (LogicalTime*)malloc(sizeof(LogicalTime));
	if (atc->lc == NULL)
	{
		free (atc);
		atc = NULL;

		return AT_LOW_MEMORY;
	}

	atc->pc = (PhysicalTime*)malloc(sizeof(PhysicalTime));
	if (atc->pc == NULL)
	{
		free (atc->lc);
		free (atc);
		atc = NULL;

		return AT_LOW_MEMORY;
	}

	resetPC();
	resetLC();

	return AT_SUCCESS;
}

ATReturn uninitATClock ()
{
	if (atc == NULL)
		return AT_NOT_INITIALIZED;

	if (atc->pc != NULL)
		free (atc->pc);
	if (atc->lc != NULL)
		free (atc->lc);
	free (atc);

	atc = NULL;

	return AT_SUCCESS;
}