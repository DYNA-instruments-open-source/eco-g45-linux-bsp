/************************************************************************/
/* Copyright (C) 2000 - 2011 Garz&Fricke GmbH							*/
/*		No use or disclosure of this information in any form without	*/
/*		the written permission of the author							*/
/************************************************************************/

#include "iptest.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

/*****************************************************************
 *Function name	: SendThreadd
 * @brief	Sender thread; responsible sending packets on the
 *			network. Received initialized packets from InitThread(),
 *			sends them and hands the sent packets over to RecvThread().
 *
 * @param	None.
 *
 * @return	None.
 */
void* SendThread()
{
	int nPacketsToSend = param_numpackets;
	int i, r, fragLen, numFrags;
	// unsigned long int bytesSent;
	DATA_HEADER_T *pPacket;			//Pointer to begin of current packet
	char* buf = NULL;			//Pointer used to slice packets into pieces.

	struct msghdr msg;
	struct iovec* pPacketFragments = 
		(struct iovec*) calloc(param_maxfragments, sizeof(struct iovec));	// array to describe fragments of a packet

	srand(timeNowMSec());

	/* Wait for Statistics Thread */
	pthread_barrier_wait(&statInitializedBarrier);

	out(3, "SendThread started");

	// log the starting time of the sender thread
	pthread_mutex_lock(&statTotalMutex);
	statTotal.tickTxStart = timeNowMSec(); 
	pthread_mutex_unlock(&statTotalMutex);
	
	while (nPacketsToSend) {

		// fetch an initialized packet from the send-stack
		pPacket = (DATA_HEADER_T *)PopStack(pSendStack);
		buf = (char *)pPacket;
		out(3, "SendThread: sending 0x%08x", pPacket);

		// initialize its tickcount
		pPacket->tickcount = timeNowMSec(); 

		// push it onto the receiver-stack
		PushStack(pRecvStack, pPacket);

		// Note: yes, it is by intention that a packet is scheduled for
		// reception before it has been sent out, because otherwise the
		// receiver might get the packet over its socket before it is
		// on the RecvStack. This results in 'dead' packets on the RecvStack
		// that will be removed as false timed-out packets, by the receiver.

		out(3, "SendThread: scheduled 0x%08x", pPacket);
				
		/* Message preparation: IOVec */
		memset(pPacketFragments, 0, param_maxfragments*sizeof(struct iovec));
		numFrags = rand() % param_maxfragments + 1;
		fragLen = param_packetsize / numFrags;
		for (i = 0; i < (numFrags-1); i++) {
			pPacketFragments[i].iov_len = fragLen;
			pPacketFragments[i].iov_base = buf;
			buf += fragLen;
		}
		pPacketFragments[i].iov_len = param_packetsize - (fragLen * i);
		pPacketFragments[i].iov_base = buf;		
	
		/* Message preparation: Message Header */
		msg.msg_name = addrInfo->ai_addr;
		msg.msg_namelen = sizeof(struct sockaddr_in);
		msg.msg_iov = pPacketFragments;
		msg.msg_iovlen = numFrags;
		//msg.msg_accrights = NULL;            		/* irrelevant to AF_INET */
		//msg.msg_accrightslen = 0;            		/* irrelevant to AF_INET */

		/* Send Message*/
		r = sendmsg(sock, &msg, 0);

		if (r == -1) {
			out(2, "SendThread failed sending packet #%u (0x%08x), errno: %i, %s",
					pPacket->sequence_id, pPacket->packet_id, &errno, strerror(errno));
		} else {
		
			// update statistics
			pthread_mutex_lock(&statTotalMutex);
			statTotal.nPacketsTx += 1;
			statTotal.nBytesTx += param_packetsize;
			pthread_mutex_unlock(&statTotalMutex);

			out(3, "SendThread: sent packet #%u(0x%08x)", pPacket->sequence_id, pPacket->packet_id);
		}
		

		// delay before sending next packet, if requested
		if (param_packetdelay)
			usleep(param_packetdelay*1000);

		// keep track of the packets still to be sent
		nPacketsToSend--;
	}
	pthread_mutex_lock(&statTotalMutex);
	statTotal.tickTxEnd = timeNowMSec(); 
	pthread_mutex_unlock(&statTotalMutex);

	out(3, "SendThread finished");
	
	return NULL;
}
