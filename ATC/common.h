#ifndef __COMMON_H__
#define __COMMON_H__

// includes
#include <stdio.h>
#include <stdlib.h>

// defines
#define AT_MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

// typedefs
typedef enum
{
	false,
	true
}bool;

typedef enum
{
	AT_FAIL,
	AT_SUCCESS,
	AT_ALREADY_INITIALIZED,
	AT_UN_INITIALIZED,
	AT_LOW_MEMORY,
	AT_NULL_PARAM
}ATReturn;

#endif __COMMON_H__