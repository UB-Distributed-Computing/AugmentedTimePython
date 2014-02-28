#ifndef __STACK_H__
#define __STACK_H__

// includes
#include "common.h"

// typedefs
typedef struct atStackNode
{
    void* data;
    struct atStackNode* next;
}ATStackNode;

typedef struct atStack
{
    ATStackNode* head;
    unsigned int count;
}ATStack;

//declarations
ATReturn createATStackNode (ATStackNode **ppNode, void *data);
ATReturn freeATStackNode (ATStackNode *node);
ATReturn createATStack (ATStack **ppS);
ATReturn freeATStack (ATStack *pS);
ATReturn ATStackPush (ATStack *s, void *data);
ATReturn ATStackPop (void **ppData, ATStack *s);
ATReturn ATStackTop (void **ppData, ATStack *s);
ATReturn ATStackCount (unsigned int *count, ATStack *s);

#endif /* __STACK_H__ */
