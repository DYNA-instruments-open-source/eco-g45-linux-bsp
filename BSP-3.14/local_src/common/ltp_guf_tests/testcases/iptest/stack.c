/************************************************************************/
/* Copyright (C) 2000 - 2011 Garz&Fricke GmbH							*/
/*		No use or disclosure of this information in any form without	*/
/*		the written permission of the author							*/
/************************************************************************/

#include "stack.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

/*****************************************************************
 *Function name	: DeinitStack
*/
STACK_T *DeinitStack(STACK_T *pStack)
{
	if (!pStack)
		return NULL;

	if (pStack->pData)
		free(pStack->pData);

	pthread_cond_destroy(&pStack->sigFreeSlot);
	pthread_cond_destroy(&pStack->sigItemAvailable);
	pthread_mutex_destroy(&pStack->mutex);
	free(pStack);

	return NULL;
}

/*****************************************************************
 *Function name	: InitStack
*/
STACK_T *InitStack(int nElements)
{
	// Allocate space for our administrative data
	STACK_T *pStack = (STACK_T *)malloc(sizeof(STACK_T));
	memset(pStack, 0, sizeof(STACK_T));	
	if (!pStack)
		return NULL;

	// Allocate space for actual data array
	pStack->pData = (void **)calloc(nElements, sizeof(void *));
	memset(pStack->pData, 0, nElements*sizeof(void *));	
	if (!pStack->pData)
	{
		free(pStack);
		return NULL;
	}
	pStack->nMaxElements = nElements;
	pStack->nCurElements = 0;

	pthread_mutex_init(&pStack->mutex, NULL);

	pthread_cond_init(&pStack->sigFreeSlot, NULL);
	pthread_cond_init(&pStack->sigItemAvailable, NULL);

	return pStack;
}

/*****************************************************************
 *Function name	: PopStack
*/
void *PopStack(STACK_T *pStack)
{
	void *pElement = NULL;

	// If stack is NULL, there is nothing to do
	if (!pStack)
		return NULL;

	pthread_mutex_lock(&pStack->mutex);

	/* If Stack is empty, wait for someone to fill it. */
	if (pStack->nCurElements == 0) {
		pthread_cond_wait(&pStack->sigItemAvailable, &pStack->mutex);
	}
	
	// Fetch element from stack
	pElement = pStack->pData[--pStack->nCurElements];
	pthread_cond_signal(&pStack->sigFreeSlot);

	pthread_mutex_unlock(&pStack->mutex);

	return pElement;
}

/*****************************************************************
 *Function name	: FilterStack
*/
int FilterStack(STACK_T *pStack,
				bool (*pFilterFunc)(void *pParam1, void *pElement, void *pParam2),
				void *pParam1, void *pParam2)
{
	int i, j;
	int nFiltered = 0;

	// If stack is NULL, there is nothing to do
	if (!pStack || !pFilterFunc)
		return nFiltered;

	pthread_mutex_lock(&pStack->mutex);

	// iterate over all elements currently on stack
	for (i = 0; i < pStack->nCurElements; i++)
	{
		// should the current element be filtered out?
		if (pFilterFunc(pParam1, pStack->pData[i], pParam2))
		{
			nFiltered++;

			// yep, remove it by copying all following
			// elements one level towards top-of-stack
			for (j = i; j < pStack->nCurElements-1; j++)
				pStack->pData[j] = pStack->pData[j+1];

			// NOTE: When we have just removed an element, we have to
			// revisit the current element index again, as we just
			// overwrote the current element with the next one.
			// We therefore have to explicitely modify both the
			// loop condition as well as the loop index of the for-loop
			// iterating over the array here!
			i--;
			pStack->nCurElements--;

			// As we removed one element, the stack will
			// have at least one free slot now
			pthread_cond_signal(&pStack->sigFreeSlot);

			if (pStack->nCurElements == 0)
			{
				break;
			}
		}
	}

	pthread_mutex_unlock(&pStack->mutex);
	return nFiltered;
}

/*****************************************************************
 *Function name	: PopStack
*/
void *FetchStack(STACK_T *pStack, void *pElement)
{
	int i;

	// If stack is NULL, there is nothing to do
	if (!pStack || !pElement)
		return NULL;

	pthread_mutex_lock(&pStack->mutex);

	/* If Stack is empty, wait for someone to fill it. */
	if (pStack->nCurElements == 0) {
		pthread_cond_wait(&pStack->sigItemAvailable, &pStack->mutex);
	}

	// Search for element on stack
	for (i = 0; i < pStack->nCurElements; i++)
	{
		if (pStack->pData[i] == pElement)
			break;
	}

	if (i == pStack->nCurElements)
	{
		// element was not found, return NULL
		pElement = NULL;
	}
	else
	{
		// element was found, remove it by copying all following
		// elements one level towards top-of-stack
		for (; i < pStack->nCurElements-1; i++)
			pStack->pData[i] = pStack->pData[i+1];
		pStack->nCurElements--;

		// As we removed one element, the stack will
		// have at least one free slot now
		pthread_cond_signal(&pStack->sigFreeSlot);
	}

	pthread_mutex_unlock(&pStack->mutex);

	return pElement;
}

/*****************************************************************
 *Function name	: PushStack
*/
void PushStack(STACK_T *pStack, void *pElement)
{
	// If stack is NULL, there is nothing to do
	if (!pStack)
		return;

	pthread_mutex_lock(&pStack->mutex);

	/* Stack full, wait for someone to free a slot. */
	if (pStack->nCurElements == pStack->nMaxElements){
		pthread_cond_wait(&pStack->sigFreeSlot, &pStack->mutex);
	}
	
	// There is space: add element to stack
	pStack->pData[pStack->nCurElements++] = pElement;
	pthread_cond_signal(&pStack->sigItemAvailable);

	pthread_mutex_unlock(&pStack->mutex);
}
