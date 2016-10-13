/************************************************************************/
/* Copyright (C) 2000 - 2011 Garz&Fricke GmbH							*/
/*		No use or disclosure of this information in any form without	*/
/*		the written permission of the author							*/
/************************************************************************/

#include "iptest.h"

#include <pthread.h>

/*****************************************************************
 *Function name	: InitThread
 * @brief	Initialization thread; responsible for initializing
 *			the contents of packets to be sent out (packet transmission
 *			is done by SendThread() and RecvThread()).
 *
 * @param	None.
 *
 * @return	None.
 */
void* InitThread()
{
	int nPacketsToGenerate = param_numpackets;
	unsigned long i;

	/* Wait for Statistics Thread */
	pthread_barrier_wait(&statInitializedBarrier);
	
	out(3, "InitThread started");
	
	while (nPacketsToGenerate) {
		DATA_HEADER_T *pPacket;
		unsigned long ulChkSum = 0;

		// fetch a free packet
		pPacket = (DATA_HEADER_T *)PopStack(pFreeStack);
		unsigned long *pRawPacket = &pPacket->sequence_id;

		out(3, "InitThread: initializing 0x%08x\r\n", pPacket);

		// initialize its data

		// Note that tickcount must not be included in checksum
		for (i = 0; i < (param_packetsize>>2)-1; i++)
			pRawPacket[i] = i;
		pPacket->sequence_id = nPacketsToGenerate;
		pPacket->magic = PACKET_MAGIC;
		pPacket->packet_id = &pRawPacket[-1];
		pPacket->length = param_packetsize;
		pPacket->chksum = 0;
		for (i = 0; i < (param_packetsize>>2)-1; i++)
			ulChkSum ^= pRawPacket[i];
		pPacket->chksum = ulChkSum;

		// put it on the send-stack to schedule it for sending
		PushStack(pSendStack, pPacket);
		out(3, "InitThread: scheduled 0x%08x\r\n", pPacket);

		// keep count of the number of packets we still have to generate
		nPacketsToGenerate--;
	}
	out(3, "InitThread finished");
	
	return NULL;
}
