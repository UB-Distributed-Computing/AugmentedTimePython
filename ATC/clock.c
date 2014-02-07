// includes
#include "clock.h"

// globals
ATClock *atc = NULL;

// definitions
at_time getLC ()
{
	if (atc == NULL)
		return 0;

	if (atc->lc == NULL)
		return 0;

	return atc->lc->time;
}

at_time getLCCount ()
{
	if (atc == NULL)
		return 0;

	if (atc->lc == NULL)
		return 0;

	return atc->lc->count;
}

at_time getPC ()
{
	if (atc == NULL)
		return 0;

	if (atc->pc == NULL)
		return 0;

	return atc->pc->time;
}

void setLC (at_time time)
{
	if (atc == NULL)
		return;

	if (atc->lc == NULL)
		return;

	atc->lc->time = time;
}

void setLCCount (at_time count)
{
	if (atc == NULL)
		return;

	if (atc->lc == NULL)
		return;

	atc->lc->count = count;
}

void setPC (at_time time)
{
	if (atc == NULL)
		return;

	if (atc->pc == NULL)
		return;

	atc->pc->time = time;
}

ATReturn resetPC ()
{
	if (atc == NULL)
		return AT_NULL_PARAM;

	if (atc->pc == NULL)
		return AT_FAIL;
	
	// TODO: resetting time means querying time from NTP?
	atc->pc->time = 0;

	return AT_SUCCESS;
}

ATReturn resetLC ()
{
	if (atc == NULL)
		return AT_UN_INITIALIZED;

	if (atc->lc == NULL || atc->pc == NULL)
		return AT_FAIL;

	// TODO: resetting logical clock means setting it as PC?
	atc->lc->time = atc->pc->time;
	atc->lc->count = 0;

	return AT_SUCCESS;
}

ATReturn initATClock ()
{
	if (atc != NULL)
		return AT_UN_INITIALIZED;

	atc = (ATClock*)malloc(sizeof(ATClock));
	if (atc == NULL)
		return AT_LOW_MEMORY;

	atc->lc = (LogicalClock*)malloc(sizeof(LogicalClock));
	if (atc->lc == NULL)
	{
		free (atc);
		atc = NULL;

		return AT_LOW_MEMORY;
	}

	atc->pc = (PhysicalClock*)malloc(sizeof(PhysicalClock));
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
		return AT_UN_INITIALIZED;

	if (atc->pc != NULL)
		free (atc->pc);
	if (atc->lc != NULL)
		free (atc->lc);
	free (atc);

	atc = NULL;

	return AT_SUCCESS;
}