#ifndef __COMMON_H__
#define __COMMON_H__

// includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

// defines
#define LOG printf
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
    #define OS_WIN
#endif

#define AT_MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

// typedefs
#ifdef OS_WIN
typedef unsigned long long uint64_t;
#endif /* OS_WIN */

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
    AT_NOT_INITIALIZED,
    AT_LOW_MEMORY,
    AT_NULL_PARAM
}ATReturn;

#endif /* __COMMON_H__ */
