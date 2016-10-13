/************************************************************************/
/* Copyright (C) 2000 - 2011 Garz&Fricke GmbH							*/
/*		No use or disclosure of this information in any form without	*/
/*		the written permission of the author							*/
/************************************************************************/

#include <stdbool.h>
#include <pthread.h>

/************************************************************************/
/*	Prototypes															*/
/************************************************************************/

// Structure describin each stack (access is supposed to happen _only_
// via functions provided by this library). A stack can save an arbitrary
// number of void pointers, only.
typedef struct
{
	void **pData;						// the actual data, currently stored as fixed-size array
	pthread_mutex_t mutex;				// a critical section to synchronize multiple threads accessing a single stack
	pthread_cond_t sigFreeSlot;			// a (manual) event providing information about the contents of the stack
	pthread_cond_t sigItemAvailable;	// a (manual) event providing information about the contents of the stack
	int nMaxElements;					// maximum capacity of the stack
	int nCurElements;					// current number of used 'slot' of the stack
} STACK_T;

/*****************************************************************
 *Function name	: InitStack
*/
/**
 * @brief	This function generates a new stack with a capacity of
 *			nElements slots.
 *
 * @param	nElements	maximum number of elements that can be
 *						stored in the stack
 *
 * @return	NULL on error, valid STACK_T pointer otherwise
 */
STACK_T *InitStack(int nElements);

/*****************************************************************
 *Function name	: DeinitStack
*/
/**
 * @brief	This function frees a stack previously allocated with InitStack
 *
 * @param	pStack	pointer to stack to be freed (may also be NULL)
 *
 * @return	always NULL
 */
STACK_T *DeinitStack(STACK_T *pStack);

/*****************************************************************
 *Function name	: PopStack
*/
/**
 * @brief	Gets (and removed) any valid element from the stack.
 *			If the stack is empty, waits for data to be put into
 *			the stack before returning.
 *
 * @param	pStack		pointer to stack
 *
 * @return	NULL on failure due to an invalid stack, otherwise
 *			_always_ a valid stack element.
 */
void *PopStack(STACK_T *pStack);

/*****************************************************************
 *Function name	: PushStack
*/
/**
 * @brief	Puts a new element into the stack. If stack is already
 *			full, waits for space to become available.
 *
 * @param	pStack		pointer to stack
 *
 * @return	None.
 */
void PushStack(STACK_T *pStack, void *pElement);

/*****************************************************************
 *Function name	: FetchStack
*/
/**
 * @brief	Fetches an element from stack that is not TOS.
 *
 * @param	pStack		pointer to stack
 * @param	pElement	element to find and remove from stack
 *
 * @return	None.
 */
void *FetchStack(STACK_T *pStack, void *pElement);

/*****************************************************************
 *Function name	: FilterStack
*/
/**
 * @brief	Iterate over all elements currently on stack. For
 *			each element on stack execute:
 *			pFilterFunc(pParam1, pElement, pParam2).
 *			If pFilterFunc() returns TRUE, the respective
 *			element is removed from the stack.
 *			If stack is currently empty, FilterStack returns
 *			without doing anything.
 *
 * @param	pStack		pointer to stack
 * @param	pFilterFunc	function to call for each element
 * @param	pParam1		1. parameter to pass to pFilterFunc
 * @param	pParam2		2. parameter to pass to pFilterFunc
 *
 * @return	Number of packets filtered, i.e. removed from stack
 */
int FilterStack(STACK_T *pStack,
				bool (*pFilterFunc)(void *pParam1, void *pElement, void *pParam2),
				void *pParam1, void *pParam2);
