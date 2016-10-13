/************************************************************************/
/* Copyright (C) 2000 - 2011 Garz&Fricke GmbH							*/
/*		No use or disclosure of this information in any form without	*/
/*		the written permission of the author							*/
/************************************************************************/

#include "stack.h"

#include <stdbool.h>
#include <pthread.h>

/************************************************************************/
/*	Data Structures							                            */
/************************************************************************/

// Header for the data packets we'll send
typedef struct
{
	unsigned long	tickcount;		// TickCounter when packet was sent
	unsigned long	sequence_id;	// A unique ID for each packet
	unsigned long	magic;			// A magic cookie to recognize packets
	unsigned long	*packet_id;		// The packet ID (only unique for packets in-flight)
	unsigned long	length;			// length of data packet (incl. header)
	unsigned long	chksum;			// XOR-checksum of DWORD in packet (excl. tickcount)
} DATA_HEADER_T;

typedef struct
{
	unsigned long	tickStart;		// TickCounter when statistics where started
	unsigned long	tickEnd;		// TickCounter when statistics where stopped
	unsigned long	tickTxStart;	// TickCounter when TX-Thread was started
	unsigned long	tickTxEnd;		// TickCounter when TX-Thread was stopped
	unsigned long	tickRxStart;	// TickCounter when RX-Thread was started
	unsigned long	tickRxEnd;		// TickCounter when RX-Thread was stopped
	unsigned long	tickSnapshot;	// TickCounter when snapshot of statiatics was taken
	unsigned long	nPacketsTx;		// Number of packets sent
	unsigned long	nBytesTx;		// Number of bytes sent
	unsigned long	nPacketsRx;		// Number of packets received (total)
	unsigned long	nBytesRx;		// Number of bytes received (total)
	unsigned long	nPacketsOk;		// Number of valid packets received in time
	unsigned long	nBytesOk;		// Number of bytes in valid packets received
	unsigned long	nPacketsCorrupt;// Number of corrupt packets
	unsigned long	nBytesCorrupt;	// Number of bytes in corrupt packets
	unsigned long	nPacketsTimeout;// Number of packets not received in time
	unsigned long	nBytesTimeout;	// Number of bytes in packets not received in time
} PACKET_STATISTICS_T;

/************************************************************************/
/*	Defines			                                                    */
/************************************************************************/
#define VER_IPTEST_MAJ			1
#define VER_IPTEST_MIN			1
#define VER_IPTEST_BLDSTR		L"SVN: $Revision: 312 $"

#define MAX_STRING				200

#define PACKET_MAGIC			0x08154711

#define MIN_PACKET_SIZE			((68 < sizeof(DATA_HEADER_T)) ? sizeof(DATA_HEADER_T) : 68)
#define MAX_PACKET_SIZE			65400

#define USE_DEFAULT_PROTOCOL	0
#define USE_UDP_PROTOCOL		1
#define USE_TCP_PROTOCOL		2
#define USE_ICMP_PROTOCOL		3

#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif

/************************************************************************/
/*	Global Variables                                                    */
/************************************************************************/

// parameter (from command-line) that determine what we're going to do;
extern char *param_szTarget;					// Address of peer (string)
extern unsigned long param_packetsize;			// Size of packets to send
extern unsigned long param_numpackets;			// Number of packets to send
extern unsigned long param_queuelength;			// Maximum packets to keep in-flight
extern unsigned long param_packettimeout;		// Maximum timeout to wait for packets to arrive (in ms)
extern unsigned long param_packetdelay;			// Minimum delay between sending packets (in ms)
extern unsigned long param_protocol;			// Protocol to use
extern unsigned long param_verbosity;			// Verbosity level of console-outputs
extern unsigned long param_statinterval;		// Intervall for priniting statistics (in ms)
extern unsigned long param_errorrate;			// Maximum error-rate (in 1/1000) that is considered okay
extern unsigned long param_maxfragments;		// Maximum number of fragment to split out-going packets into

/* WORKING ENVIRONMENT */
extern pthread_t workerThreads[3];	 			//The three worker threads. Statistics isn't work.
extern pthread_t statThread;

extern int sock;								// The Socket
extern struct addrinfo* addrInfo;				// Structure with address information of our echo peer. 

/**
 * In three thread-safe stacks we shovel around "tickets", e.g., pointers to packets. 
 * Each thread pops from one stack packets it is supposed to work with, and pushes them onto another if it's done.
 * The packets themselves remain in the buffer pPacketBuffer points to at all times.
 * This is to ensure that at all times there exists only one specific number of packets in the whole program.
 */
extern unsigned char *pPacketBuffer;	 	// Buffer to store all packets we handle

extern STACK_T *pFreeStack;		// buffer-stack for "tickets for" (pointers to) free packets (popped by InitThread, pushed by RecvThread)
extern STACK_T *pSendStack;		// buffer-stack for "tickets for" (pointers to) packets to be sent (popped by SendThread, pushed by InitThread)
extern STACK_T *pRecvStack;		// buffer-stack for "tickets for" (pointers to) packets to be received (popped by RecvThread, pushed by SendThread)

/* Barrier: Statistics were initialized, all threads start working. */
extern pthread_barrier_t statInitializedBarrier;

/* Signal: Worker threads done, StstThread post final statistics and exit, too. */
extern pthread_cond_t workersDoneCond;
extern pthread_mutex_t workersDoneMutex;

/* Statistics board. Worker threads write, StatThread outputs differences since last statistics output interval and total stats on exit.*/ 
extern pthread_mutex_t statTotalMutex;
extern PACKET_STATISTICS_T statTotal;			// current set of statistics variables

/************************************************************************/
/*	Functions															*/
/************************************************************************/

/*****************************************************************
 *Function name	: out
*/
/**
 * @brief	Outputs messages on console incl. program and target
 *			information.
 *
 * @param	verbosity	Output message only if current verbosity
 *						is larger or equal to 'verbosity'
 * @param	fmt, ...	Standard printf-parameters
 *
 * @return	None.
 */
extern void out(unsigned long verbosity, char const* fmt, ...);

/*****************************************************************
 *Function name	: out_naked
*/
/**
 * @brief	Outputs messages on console without program and target
 *			information.
 *
 * @param	verbosity	Output message only if current verbosity
 *						is larger or equal to 'verbosity'
 * @param	fmt, ...	Standard printf-parameters
 *
 * @return	None.
 */
extern void out_naked(unsigned long verbosity, char const* fmt, ...);

/**
 * Setup the environment.
 */ 
extern int setup();

/**
* Gives the current tick in Milliseconds. Wraps after about 47 days.
*/
extern unsigned long int timeNowMSec();

/**
* Deinitializes everything we used.
*/
extern void cleanup();

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
extern void* InitThread();

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
extern bool RecvBufferFilter(void *pParam1, void *pElement, void *pParam2);

/*****************************************************************
 * Function name	: RecvThread
 * @brief	Receiver thread; responsible receiving and checking
 *			packets from network. Hands received packets back
 *			to InitThread() to be reused.
 *
 * @param	None
 *
 * @return	None.
 */
extern void* RecvThread();

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
extern void* SendThread();

/*****************************************************************
 *Function name	: ShowStatistics
*/
/**
 * @brief	Outputs statistics information on console
 *
 * @param	pStat	Pointer to statistics-structure to be output
 *
 * @return	None.
 */
extern void showStatistics(int verbosity, PACKET_STATISTICS_T *pStat);

extern void showDifferentialStatistics(PACKET_STATISTICS_T* statThisTime, PACKET_STATISTICS_T* statLastTime);

extern int statisticThreadWait();

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
extern void* StatThread(void *ulErrorRate);