#ifndef __STACK_H__
#define __STACK_H__

// includes
#include "common.h"

// typedefs
typedef struct atcStackNode
{
	void *data;
	struct atcStackNode* next;
}ATCStackNode;

typedef struct atcStack
{
	ATCStackNode* head;
}ATCStack;

//declarations
ATCStackNode* createATCStackNode (void *data);
void freeATCStackNode (ATCStackNode *node);
void createATCStack (ATCStack **ppS);
void freeATCStack (ATCStack **ppS);
void ATCStackPush (ATCStack *s, void *data);
void* ATCStackPop (ATCStack *s);
void* ATCStackTop (ATCStack *s);

#endif __STACK_H__