#include "clock.h"

// globals
ATClock *atc = NULL;

// definitions
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