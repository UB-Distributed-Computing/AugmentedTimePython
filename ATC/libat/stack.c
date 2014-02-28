// includes
#include "stack.h"

// definitions
ATReturn createATStackNode (ATStackNode **ppNode, void *data)
{
    ATStackNode *newNode;

    if (ppNode == NULL)
        return AT_NULL_PARAM;

    newNode = (ATStackNode*)malloc(sizeof(ATStackNode));

    if (newNode == NULL)
        return AT_LOW_MEMORY;

    newNode->data = data;
    newNode->next = NULL;
    *ppNode = newNode;

    return AT_SUCCESS;
}

ATReturn freeATStackNode (ATStackNode *node)
{
    if (node != NULL)
        free (node);

    return AT_SUCCESS;
}

ATReturn createATStack (ATStack **ppS)
{
    ATStack *newStack;

    if (ppS == NULL)
        return AT_NULL_PARAM;

    newStack = (ATStack*)malloc(sizeof(ATStack));

    if (newStack == NULL)
        return AT_LOW_MEMORY;

    newStack->head = NULL;
    newStack->count = 0;
    *ppS = newStack;

    return AT_SUCCESS;
}

ATReturn freeATStack (ATStack *pS)
{
    if (pS == NULL)
        return AT_NULL_PARAM;

    free (pS);

    return AT_SUCCESS;
}

ATReturn ATStackPush (ATStack *s, void *data)
{
    ATStackNode *node;
    ATReturn errorCode = AT_SUCCESS;

    if (s == NULL || data == NULL)
        return AT_NULL_PARAM;

    errorCode = createATStackNode (&node, data);
    if (errorCode != AT_SUCCESS)
        return errorCode;

    node->next = s->head;
    s->head = node;
    s->count++;

    return AT_SUCCESS;
}

ATReturn ATStackPop (void **ppData, ATStack *s)
{
    ATStackNode *next;
    ATReturn errorCode = AT_SUCCESS;

    if (ppData == NULL || s == NULL)
        return AT_NULL_PARAM;

    if (s->head == NULL)
    {
        *ppData = NULL;
    }
    else
    {
        *ppData = s->head->data;
        next = s->head->next;
        errorCode = freeATStackNode (s->head);
        s->head = next;
        s->count--;
    }

    return errorCode;
}

ATReturn ATStackTop (void **ppData, ATStack *s)
{
    if (ppData == NULL || s == NULL)
        return AT_NULL_PARAM;

    if (s->head == NULL)
        *ppData = NULL;
    else
        *ppData = s->head->data;

    return AT_SUCCESS;
}

ATReturn ATStackCount (unsigned int *count, ATStack *s)
{
    if (s == NULL || count == NULL)
        return AT_NULL_PARAM;

    *count = s->count;

    return AT_SUCCESS;
}
