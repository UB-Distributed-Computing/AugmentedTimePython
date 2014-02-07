// includes
#include "stack.h"

// definitions
ATCStackNode* createATCStackNode (void *data)
{
	ATCStackNode *newNode = (ATCStackNode *)malloc(sizeof(ATCStackNode));

	if (newNode != NULL)
	{
		newNode->data = data;
		newNode->next = NULL;
	}

	return newNode;
}

void freeATCStackNode (ATCStackNode *node)
{
	if (node != NULL)
		free (node);
}

void createATCStack (ATCStack **ppS)
{
	*ppS = (ATCStack*)malloc(sizeof(ATCStack));
	(*ppS)->head = NULL;
}

void freeATCStack (ATCStack **ppS)
{
	if (ppS == NULL)
		return;

	if (*ppS == NULL)
		return;

	free (*ppS);
	*ppS = NULL;
}

void ATCStackPush (ATCStack *s, void *data)
{
	ATCStackNode *node;

	if (s == NULL || data == NULL)
		return;

	node = createATCStackNode (data);
	if (node == NULL)
		return;

	node->next = s->head;
	s->head = node;
}

void* ATCStackPop (ATCStack *s)
{
	ATCStackNode *next;
	void *retData;

	if (s == NULL)
		return NULL;

	if (s->head == NULL)
		return NULL;

	retData = s->head->data;
	next = s->head->next;
	freeATCStackNode (s->head);
	s->head = next;

	return retData;
}

void* ATCStackTop (ATCStack *s)
{
	if (s == NULL)
		return NULL;

	if (s->head == NULL)
		return NULL;

	return s->head->data;
}