/************************************************************************/
/* Copyright (C) 2000 - 2011 Garz&Fricke GmbH							*/
/*		No use or disclosure of this information in any form without	*/
/*		the written permission of the author							*/
/************************************************************************/

#include "iptest.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdbool.h>
#include <pthread.h>
#include <netinet/in.h>
#include <signal.h>

/*****************************************************************
 * Function name	: RecvBufferFilter
 * @brief	Helper function for RecvThread() to filter list of
 *			packets expected to arrive. Can remove one matching packet
 *			(e.g. after it has been successfully received) and
 *			removes timed-out packets from the list of expected
 *			packets.
 *			Will be called by RecvThread() through call to FilterStack()
 *			iterating over all packets on a stack.
 *
 * @param	pParam1		Stack to push freed packets on (usually pFreeStack)
 * @param	pElement	Stack-element currently to be evaluated, whether
 *						is may stay on the stack or must be removed.
 * @param	pParam2		A reference element that will be compared with
 *						pElement. If both are equal (i.e. the reference
 *						element is found on the stack) pElement will
 *						be removed.
 *
 * @return	true if pElement must be removed from the stack, false if it
 *			may stay.
 */
bool RecvBufferFilter(void *pParam1, void *pElement, void *pParam2)
{
	STACK_T *pStack = (STACK_T *)pParam1;
	DATA_HEADER_T *pCurPacket = (DATA_HEADER_T *)pElement;
	DATA_HEADER_T *pRefPacket = (DATA_HEADER_T *)pParam2;

	bool bRemovePacket = false;

	if (!pStack || !pCurPacket)
		return false;

	// if a reference packet is given, and it matches the current packet,
	// mark the current packet for removal.
	if (pRefPacket &&
			(pCurPacket->packet_id == pRefPacket->packet_id))
	{
		bRemovePacket = true;
		out(3, "RecvThread: freed #%u 0x%08x", pCurPacket->sequence_id, pCurPacket);
	}
	
	// if the current packet timed-out, mark it for removal
	// and update statistics
	if ((timeNowMSec() - pCurPacket->tickcount) > param_packettimeout) {
		pthread_mutex_lock(&statTotalMutex);
		statTotal.nPacketsTimeout += 1;
		statTotal.nBytesTimeout += param_packetsize;
		pthread_mutex_unlock(&statTotalMutex);

		bRemovePacket = true;
		out(3, "RecvThread: freed #%u 0x%08x (timeout)", pCurPacket->sequence_id, pCurPacket);
	}

	if (bRemovePacket) {
		PushStack(pStack, pCurPacket);
		return true;
	}


	return false;
}

/*****************************************************************
 * Function name	: RecvThread
 * @brief	Receiver thread; responsible receiving and checking
 *			packets from network. Hands received packets back
 *			to InitThread() to be reused.
 *
 * @param	None.
 *
 * @return	None.
 */
void* RecvThread()
{
	unsigned long i;
	unsigned long nPacketsToRecv = param_numpackets;
	DATA_HEADER_T *pRecvData = NULL;
	unsigned long *pRawPacket;
	int nTimeoutCounter = 0;
	fd_set fdReadSockets;
	struct timeval tvReadTimeout;
	unsigned long ulChkSum = 0;

	// allocate space for incoming packets
	pRecvData = (DATA_HEADER_T *)malloc(param_packetsize);
	if (!pRecvData)
		nPacketsToRecv = 0;

	/* Wait for Statistics Thread */
	pthread_barrier_wait(&statInitializedBarrier);

	// log starting time of receiver thread
	pthread_mutex_lock(&statTotalMutex);
	statTotal.tickRxStart = timeNowMSec(); 
	pthread_mutex_unlock(&statTotalMutex);

	out(3, "RecvThread started");
	
	while (nPacketsToRecv) {
		// we want to wait for packets with a time-out, so setup
		// structures for a select call
		FD_ZERO(&fdReadSockets);
		FD_SET(sock, &fdReadSockets);
		tvReadTimeout.tv_sec = (2*param_packettimeout)/1000;
		tvReadTimeout.tv_usec = ((2*param_packettimeout)%1000)*1000;

		if (0 == select(sock + 1, &fdReadSockets, NULL, NULL, &tvReadTimeout)) {
			// Timeout; Collect garbage
			nPacketsToRecv -= FilterStack(pRecvStack, RecvBufferFilter, pFreeStack, NULL);
			nTimeoutCounter++;

			// We allow some number of timeouts to compensate for a stuck IP-stack.
			if (nTimeoutCounter < 5)
				continue;
			else
				break;
		}
		nTimeoutCounter = 0;

		// receive packet from socket
		memset(pRecvData, 0, param_packetsize);
		if (param_packetsize != (unsigned long)recvfrom(sock, (char *)pRecvData, param_packetsize, 0, NULL, NULL)) {
			out(2, "RecvThread failed receiving packet #%u (0x%08x)%u",
					pRecvData->sequence_id, pRecvData->packet_id);
		}
		pRawPacket = &(pRecvData->sequence_id);

		// update statistics
		pthread_mutex_lock(&statTotalMutex);
		statTotal.nPacketsRx += 1;
		statTotal.nBytesRx += param_packetsize;
		pthread_mutex_unlock(&statTotalMutex);

		// Note that tickcount must not be included in checksum
		for (i = 0; i < (param_packetsize>>2)-1; i++)
			ulChkSum ^= pRawPacket[i];

		// check if packet is okay 
		if (ulChkSum || (pRecvData->magic != PACKET_MAGIC)) {
			// nope, count it as corrupt and update statistics
			pthread_mutex_lock(&statTotalMutex);
			statTotal.nPacketsCorrupt += 1;
			statTotal.nBytesCorrupt += param_packetsize;
			pthread_mutex_unlock(&statTotalMutex);

			out(2, "RecvThread: Packet corrupted");
		} else {
			// yep, packet is okay, but did it arrive in time?
			
			if ((timeNowMSec() - pRecvData->tickcount) <= param_packettimeout) {
				// yep, update statistics
				pthread_mutex_lock(&statTotalMutex);
				statTotal.nPacketsOk += 1;
				statTotal.nBytesOk += param_packetsize;
				pthread_mutex_unlock(&statTotalMutex);
			}

			// remove just received packets and any old timed-out packets
			// from receive buffer
			nPacketsToRecv -= FilterStack(pRecvStack, RecvBufferFilter, pFreeStack, pRecvData->packet_id);
			out(3, "RecvThread: got packet #%u(0x%08x)", pRecvData->sequence_id, pRecvData->packet_id);
		}
	}

	// receive thread finished; add any remaining packets to the timed-out packets
	pthread_mutex_lock(&statTotalMutex);
	statTotal.nPacketsTimeout += nPacketsToRecv;
	statTotal.nBytesTimeout += (nPacketsToRecv*param_packetsize);
	statTotal.tickRxEnd = timeNowMSec(); 
	statTotal.tickTxEnd = timeNowMSec();
	pthread_mutex_unlock(&statTotalMutex);
	
	// if there were still packets pending, kill the other two threads
	// otherwise the program is never going to finish.
	if (nPacketsToRecv) {
		pthread_cancel(workerThreads[0]);	// initThread
		pthread_cancel(workerThreads[2]);	// sendThread
	}

	out(3, "RecvThread finished");
	
	return NULL;
}
