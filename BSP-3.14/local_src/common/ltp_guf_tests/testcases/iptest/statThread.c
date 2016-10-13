/************************************************************************/
/* Copyright (C) 2000 - 2011 Garz&Fricke GmbH							*/
/*		No use or disclosure of this information in any form without	*/
/*		the written permission of the author							*/
/************************************************************************/

#include "iptest.h"

#include <string.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

/*****************************************************************
 *Function name	: ShowStatistics
 * @brief	Outputs statistics information on console
 *
 * @param	pStat	Pointer to statistics-structure to be output
 *
 * @return	None.
 */
void showStatistics(int verbosity, PACKET_STATISTICS_T *pStat)
{
	unsigned long ulTicks;
	unsigned long ulRxTicks;
	unsigned long ulTxTicks;

	if (!pStat)
		return;

	// calculate duration of statistics run
	ulTicks = pStat->tickEnd - pStat->tickStart;
	ulTxTicks = pStat->tickTxEnd - pStat->tickTxStart;
	ulRxTicks = pStat->tickRxEnd - pStat->tickRxStart;

	// if duration is 0ms, we just set it to 1ms to prevent
	// division-by-zero (we can't provide accurate bandwith
	// calculations in this case anyway)
	ulTicks = (ulTicks == 0) ? 1 : ulTicks;
	ulTxTicks = (ulTxTicks == 0) ? 1 : ulTxTicks;
	ulRxTicks = (ulRxTicks == 0) ? 1 : ulRxTicks;

	out(verbosity + 1, "    Total: %u bytes in %u msecs => %u KBytes/s",
		pStat->nBytesTx + pStat->nBytesRx,
		ulTicks,
		(long)(((pStat->nBytesTx + pStat->nBytesRx)/ulTicks)/1.024f));
	out(verbosity, "    TX   : %u bytes in %u msecs => %u KBytes/s",
		pStat->nBytesTx, ulTxTicks,
		(long)((pStat->nBytesTx/ulTxTicks)/1.024f));
	out(verbosity, "    RX   : %u bytes in %u msecs => %u KBytes/s",
		pStat->nBytesRx, ulRxTicks,
		(long)((pStat->nBytesRx/ulRxTicks)/1.024f));
	out(verbosity + 1, "    RXok : %u bytes in %u msecs => %u KBytes/s",
		pStat->nBytesOk, ulRxTicks,
		(long)((pStat->nBytesOk/ulRxTicks)/1.024f));
	out(verbosity, "    RXbad: %u packets (%u bytes) corrupt, %u packets (%u bytes) timed-out",
		pStat->nPacketsCorrupt, pStat->nBytesCorrupt,
		pStat->nPacketsTimeout, pStat->nBytesTimeout);
}

void showDifferentialStatistics(PACKET_STATISTICS_T* statThisTime, PACKET_STATISTICS_T* statLastTime)
{
	PACKET_STATISTICS_T statDifference;

	/* Construct new statistics board that shows the difference between two other boards. */
	statDifference.tickStart		= statLastTime->tickSnapshot;
	statDifference.tickTxStart		= statLastTime->tickSnapshot;
	statDifference.tickRxStart		= statLastTime->tickSnapshot;
	statDifference.tickEnd			= statThisTime->tickSnapshot;
	statDifference.tickTxEnd		= statThisTime->tickSnapshot;
	statDifference.tickRxEnd		= statThisTime->tickSnapshot;
	statDifference.nPacketsTx		= statThisTime->nPacketsTx - statLastTime->nPacketsTx;
	statDifference.nBytesTx			= statThisTime->nBytesTx - statLastTime->nBytesTx;
	statDifference.nPacketsRx		= statThisTime->nPacketsRx - statLastTime->nPacketsRx;
	statDifference.nBytesRx			= statThisTime->nBytesRx - statLastTime->nBytesRx;
	statDifference.nPacketsOk		= statThisTime->nPacketsOk - statLastTime->nPacketsOk;
	statDifference.nBytesOk			= statThisTime->nBytesOk - statLastTime->nBytesOk;
	statDifference.nPacketsCorrupt	= statThisTime->nPacketsCorrupt - statLastTime->nPacketsCorrupt;
	statDifference.nBytesCorrupt	= statThisTime->nBytesCorrupt - statLastTime->nBytesCorrupt;
	statDifference.nPacketsTimeout	= statThisTime->nPacketsTimeout - statLastTime->nPacketsTimeout;
	statDifference.nBytesTimeout	= statThisTime->nBytesTimeout - statLastTime->nBytesTimeout;

	/* Output this */
	showStatistics(2, &statDifference);
}

int statisticThreadWait()
{
	int waitRes;
	struct timespec timeNextStat;
	
	memset(&timeNextStat, 0, sizeof(struct timespec)); 	// make sure the struct is empty

	/* Wait for next statistics run or end of program. */
	if(param_statinterval == 0){
		waitRes = pthread_cond_wait(&workersDoneCond, &workersDoneMutex);		// Wait till workers done, forever
	} else {
		/* Constructing the time when to wake up for next statistics run if program doesn't quit in the meantime. */
		clock_gettime(CLOCK_REALTIME, &timeNextStat);
		timeNextStat.tv_sec += param_statinterval / 1000;				// Param_statinterval is is milliseconds: Whole Seconds go here,
		timeNextStat.tv_nsec += (param_statinterval % 1000) * (long int) pow(10, 6);		// the rest, in nanoseconds, goes here.
		if (timeNextStat.tv_nsec >= (long int)pow(10, 9)) {				// Maybe, theres an overflow in the nanoseconds.
			timeNextStat.tv_sec += 1;
			timeNextStat.tv_nsec %= (long int)pow(10, 9);
		}

		//Wait till workers done or it's time to show statistics.
		waitRes = pthread_cond_timedwait(&workersDoneCond, &workersDoneMutex, &timeNextStat);
	}
	
	return waitRes;
}	

/*****************************************************************
 *Function name	: StatThread
 * @brief	Statistics thread responsible for printing statistics
 *			information in regular intervalls and before leaving
 *			the application.
 *
 * @param	pParam		unused
 *
 * @return	None.
 */
void* StatThread(void *ulErrorRate)
{
	/* Snapshots of statistics board, from last and from current time StatThread was woken. */
	PACKET_STATISTICS_T statLastWake, statCurrentWake;
	int waitRes;

	/* Statitics preparation: Other threads wait at the barrier. */
	pthread_mutex_lock(&statTotalMutex);
	memset(&statTotal, 0, sizeof(PACKET_STATISTICS_T));
	statTotal.tickStart = timeNowMSec(); 
	pthread_mutex_unlock(&statTotalMutex);

	statLastWake = statTotal;								//Get ready for first differential statistics output
	statLastWake.tickSnapshot = timeNowMSec();
	
	pthread_mutex_lock(&workersDoneMutex);					// we don't want to get signalled until we are actually 
															//  waiting for it, so lock the mutex before we start
	
	pthread_barrier_wait(&statInitializedBarrier);			//Meet other threads here. All go!
	
	out(3, "StatThread started");
	
	while(true) {

		waitRes = statisticThreadWait();

		pthread_mutex_lock(&statTotalMutex);

		//As long as this works...
		statCurrentWake = statTotal;
		statCurrentWake.tickSnapshot = timeNowMSec();

		pthread_mutex_unlock(&statTotalMutex);

		out(2, "Running statistics:");
		showDifferentialStatistics(&statCurrentWake, &statLastWake);

		statLastWake = statCurrentWake;

		// if the finish-event was signalled we're done here
		if (waitRes == 0)
			break;

	}

	// we're done. set the finishing times of the total statistics
	// to their proper values...
	pthread_mutex_lock(&statTotalMutex);
	statTotal.tickEnd = timeNowMSec();

	// ...and print them
	out(1, "Total statistics:");
	showStatistics(1, &statTotal);
	pthread_mutex_unlock(&statTotalMutex);

	// calculate error-rate (per-mille) and round-up;
	out(3, "Now manipulating ulErrorRate: ");
	*((unsigned long *)ulErrorRate) = ((1000*(statTotal.nPacketsCorrupt + statTotal.nPacketsTimeout))/param_numpackets) + 1;

	return NULL;
}
