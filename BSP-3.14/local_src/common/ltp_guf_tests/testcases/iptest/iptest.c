/************************************************************************/
/* Copyright (C) 2000 - 2011 Garz&Fricke GmbH							*/
/*		No use or disclosure of this information in any form without	*/
/*		the written permission of the author							*/
/************************************************************************/

#include "iptest.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

/************************************************************************/
/*	Global Variables: Definitions						                */
/************************************************************************/

/* Extern Global Variables */
extern int  Tst_count;             	/* counter for tst_xxx routines.         */
extern char *TESTDIR;              	/* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID     = "iptest         ";			/* test program identifier.              */
int  TST_TOTAL = 1;                 		/* total number of tests in this file.   */

// parameter (from command-line) that determine what we're going to do;
char *param_szTarget = NULL;							// Address of peer (string)
unsigned long param_packetsize = 1024;					// Size of packets to send
unsigned long param_numpackets = 1000;					// Number of packets to send
unsigned long param_queuelength = 5;					// Maximum packets to keep in-flight
unsigned long param_packettimeout = 1000;				// Maximum timeout to wait for packets to arrive (in ms)
unsigned long param_packetdelay = 0;					// Minimum delay between sending packets (in ms)
unsigned long param_protocol = USE_DEFAULT_PROTOCOL;	// Protocol to use
unsigned long param_verbosity = 1;                      // Verbosity level of console-outputs
unsigned long param_statinterval = 2000;				// Intervall for priniting statistics (in ms)
unsigned long param_errorrate = 20;						// Maximum error-rate (in 1/1000) that is considered okay
unsigned long param_maxfragments = 1;					// Maximum number of fragment to split out-going packets into


/* WORKING ENVIRONMENT */

pthread_t workerThreads[3];	 	// The three worker threads. Statistics isn't work.
pthread_t statThread;

int sock = -1;						// The Socket
struct addrinfo* addrInfo = NULL;	// Structure with address information of our echo peer. 

/**
 * In three thread-safe stacks we shovel around "tickets", e.g., pointers to packets. 
 * Each thread pops from one stack packets it is supposed to work with, and pushes them onto another if it's done.
 * The packets themselves remain in the buffer pPacketBuffer points to at all times.
 * This is to ensure that at all times there exists only one specific number of packets in the whole program.
 */
unsigned char *pPacketBuffer = NULL;	 	// Buffer to store all packets we handle

STACK_T *pFreeStack = NULL;		// buffer-stack for "tickets for" (pointers to) free packets (popped by InitThread, pushed by RecvThread)
STACK_T *pSendStack = NULL;		// buffer-stack for "tickets for" (pointers to) packets to be sent (popped by SendThread, pushed by InitThread)
STACK_T *pRecvStack = NULL;		// buffer-stack for "tickets for" (pointers to) packets to be received (popped by RecvThread, pushed by SendThread)

/* Barrier: Statistics were initialized, all threads start working. */
pthread_barrier_t statInitializedBarrier;

/* Signal: Worker threads done, StstThread post final statistics and exit, too. */
pthread_cond_t workersDoneCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t workersDoneMutex = PTHREAD_MUTEX_INITIALIZER;

/* Statistics board. Worker threads write, StatThread outputs differences since last statistics output interval and total stats on exit.*/ 
pthread_mutex_t statTotalMutex = PTHREAD_MUTEX_INITIALIZER;
PACKET_STATISTICS_T statTotal;	// current set of statistics variables

/************************************************************************/
/*	Functions															*/
/************************************************************************/

/*****************************************************************
 *Function name	: out
*/
/**
 * @brief	Outputs messages on console via LTP harness API.
 *
 * @param	verbosity	Output message only if current verbosity
 *						is larger or equal to 'verbosity'
 * @param	fmt, ...	Standard printf-parameters
 *
 * @return	None.
 */
void out(unsigned long verbosity, char const* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	
	unsigned int print_ret = 0;
	size_t msg_buf_size = 64 * sizeof(char);
	char *msg_buf = NULL;
	
	if (param_verbosity < verbosity)
		return;

	// since LTP does not provide a tst_resm(...) which accepts a va_list instead of 
	// the variadic arguments we have to compose the string first, and pass it to 
	// tst_resm() without any formats left
		
	do {
		// double the message buffer because it was too small in the last run
		// (or allocate it, if msg_buf is still NULL)
		msg_buf_size *= 2;
		
		free(msg_buf);
		msg_buf = malloc(msg_buf_size);
		
		if (!msg_buf) {
			// error!
			tst_brkm(TBROK, cleanup, "Error while trying to print TINFO message!");
		}
		
		// write formatted string to message buffer
		print_ret = vsnprintf(msg_buf, msg_buf_size, fmt, va);
		// retry the procedure if the buffer was too small
	} while(print_ret >= msg_buf_size);

	//TODO: remaining '%' characters in the string provide the possibility of errors
	// when the ltp-function tries to parse them for arguments which don't exist.
	// so we will have to escape all '%' characters to '%%' or remove them.
	// This is currently no problem as there are no messages which want to display % signs
	// via out() but should be considered when making changes

	tst_resm(TINFO, msg_buf);
	
	free(msg_buf);
}

/*****************************************************************
 *Function name	: out_naked
 * @brief	Outputs messages on console without using LTP test API
 *
 * @param	verbosity	Output message only if current verbosity
 *						is larger or equal to 'verbosity'
 * @param	fmt, ...	Standard printf-parameters
 *
 * @return	None.
 */
void out_naked(unsigned long verbosity, char const* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	
	if (param_verbosity < verbosity)
		return;
		
	vprintf(fmt, va);
}


/**
* Gives the current tick in Milliseconds. Wraps after about 47 days.
*/
unsigned long int timeNowMSec()
{
	struct timespec timeNow;

	clock_gettime(CLOCK_REALTIME, &timeNow);
	return (timeNow.tv_sec*1000 + timeNow.tv_nsec/(1000000));
}

void help()
{
	out_naked(0, "  -l x    Size of packets to send (default: %lu)\r\n", param_packetsize);
    out_naked(0, "  -n n    Total number of packets to send (default: %lu)\r\n", param_numpackets);
    out_naked(0, "  -m n    Maximum number of packets 'in-fly' (default: %lu)\r\n", param_queuelength);
    out_naked(0, "  -w x    Timeout (in ms) to wait for a packet (default: %lu)\r\n", param_packettimeout);
    out_naked(0, "  -W x    Time to wait between sending packets (default: %lu)\r\n", param_packetdelay);
    out_naked(0, "  -s x    Interval for statistics in ms (0 = only total) (default %lu)\r\n", param_statinterval);
    out_naked(0, "  -r x    Maximum error-rate (in 1/1000) considered okay (default: %lu)\r\n", param_errorrate);
    out_naked(0, "  -k n    Maximum number of data-chunks (fragments) (default: %lu)\r\n", param_maxfragments);
    out_naked(0, "  -u      Use UDP Echo service (port 7) as peer (default)\r\n");
    out_naked(0, "  -a      Use TCP Echo service (port 7) as peer\r\n");
    out_naked(0, "  -b      Use ICMP Echo service as peer.\r\n");
	out_naked(0, "  -g IP   IP of the remote computer.\r\n");
	out_naked(0, "  -v      Be verbose. This outputs statistics during the test. The interval may be specified via -s\r\n");
	out_naked(0, "  -q      Be quiet. Suppress all output except for the pass/fail message\r\n");
	
	out_naked(0, "Note: IP-stacks seem to have a maximum packet size they can handle.\r\n");
    out_naked(0, "Packets larger than that can be sent, but will not be received.\r\n");
    out_naked(0, "There seems to be no way to figure out this maximum size in software.\r\n");
    out_naked(0, "Experiments have shown different restriction on different systems:\r\n");
    out_naked(0, "  Windows XP : packets smaller than 8100 bytes seem to be safe.\r\n");
    out_naked(0, "  Windows CE : packets smaller than 5000 bytes seem to be safe.\r\n");
}

/**
 * Setup the environment.
 */ 
int setup()
{
	int i;
	unsigned long optVal;
	socklen_t optLen;

	
	//open socket.
	sock = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);

	if (sock == -1) {
		tst_brkm(TBROK, cleanup, "Failed to open socket: %i, %s \n", errno, strerror(errno));
		return 2;
	}

	// The platform might have send-/receive-buffers in the IP-stack smaller
	// than we'd like to use. Try to adjust these. (If it fails, we just
	// live with whatever the platform supplies.)
	optLen = sizeof(int);
	if (0 == getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&optVal, &optLen)) {
		if (optVal < 2*param_packetsize*param_queuelength) {
			optVal = 2*param_packetsize*param_queuelength;
			optLen = sizeof(int);
			setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&optVal, optLen);
		}
	}
	optLen = sizeof(int);
	if (0 == getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&optVal, &optLen)) {
		if (optVal < 2*param_packetsize*param_queuelength) {
			optVal = 2*param_packetsize*param_queuelength;
			optLen = sizeof(int);
			setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&optVal, optLen);
		}
	}

	/* Buffers and stacks */
	pFreeStack = InitStack(param_queuelength);
	pSendStack = InitStack(param_queuelength);
	pRecvStack = InitStack(param_queuelength);

	pPacketBuffer = (unsigned char *)calloc(param_queuelength*param_packetsize, sizeof(unsigned char));

	
	if (!pFreeStack || !pSendStack || !pRecvStack || !pPacketBuffer) {
		tst_brkm(TBROK, cleanup, "Failed to allocate packet buffers");
		return 2;
	}

	/* Make and push tickets for our packets. */
	for (i = 0; (unsigned long)i < param_queuelength; i++) {
		PushStack(pFreeStack, &pPacketBuffer[i*param_packetsize]);
	}
	

	/* Make Barrier ready */
	pthread_barrier_init(&statInitializedBarrier, NULL, 4);
	
	return 0;
}

/**
* Deinitializes everything we used.
*/
void cleanup()
{
	if (addrInfo){
		freeaddrinfo(addrInfo);
	}

	close(sock);
	
	pFreeStack = DeinitStack(pFreeStack);
	pSendStack = DeinitStack(pSendStack);
	pRecvStack = DeinitStack(pRecvStack);

	if (pPacketBuffer) {
		free(pPacketBuffer);
	}

	pthread_barrier_destroy(&statInitializedBarrier);	
	
	pthread_cond_destroy(&workersDoneCond);
	pthread_mutex_destroy(&workersDoneMutex);
	
	pthread_mutex_destroy(&statTotalMutex);
}

int main(int argc, char* argv[]) 
{
	int i, r;
	struct addrinfo addrHints;
	unsigned long ulErrorRate;
	char *protocol_name; 
	
	int opt_l = 0, opt_n = 0, opt_m = 0, opt_w = 0, opt_W = 0, opt_s = 0, opt_r = 0, opt_k = 0, opt_u = 0, opt_a = 0, opt_b = 0, opt_g = 0, opt_v = 0, opt_q = 0;
	char *str_l, *str_n, *str_m, *str_w, *str_W, *str_s, *str_r, *str_k, *str_u, *str_a, *str_b, *str_g, *msg;
	
	option_t options[] = {
		{"l:", &opt_l, &str_l},
		{"n:", &opt_n, &str_n},
		{"m:", &opt_m, &str_m},
		{"w:", &opt_w, &str_w},
		{"W:", &opt_W, &str_W},
		{"s:", &opt_s, &str_s},
		{"r:", &opt_r, &str_r},
		{"k:", &opt_k, &str_k},
		{"u", &opt_u, &str_u},
		{"a", &opt_a, &str_a},
		{"b", &opt_b, &str_b},
		{"g:", &opt_g, &str_g},
		{"v", &opt_v, NULL},
		{"q", &opt_q, NULL},
		{NULL, NULL, NULL}
	};	

	/* parse options */
	if ((msg = parse_opts(argc, argv, options, &help)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	
	if (opt_l) {
		if (sscanf(str_l, "%lu", &param_packetsize) != 1
		||	(param_packetsize < MIN_PACKET_SIZE || param_packetsize > MAX_PACKET_SIZE)) {
			param_packetsize = 1024;
			tst_resm(TWARN,
				"Bad size for packets to send. Must be %u <= <val> <= %u. Using standard value %lu now.",
				MIN_PACKET_SIZE, MAX_PACKET_SIZE, param_packetsize);
		}
	}
	if (opt_n) {
		if (sscanf(str_n, "%lu", &param_numpackets) != 1
		||	(param_numpackets < 1 || param_numpackets > INT_MAX)) {
			param_numpackets = 1000;
			tst_resm(TWARN,
				"Bad total number of packets to send. Must be %u <= <val> <= %u. Using standard value %lu now.",
				1, INT_MAX, param_numpackets);
		}
	}
	if (opt_m) {
		if (sscanf(str_m, "%lu", &param_queuelength) != 1
		||	(param_queuelength < 1 || param_queuelength > INT_MAX)) {
			param_queuelength = 5;
			tst_resm(TWARN,
				"Bad maximum number of packets 'in-fly'. Must be %u <= <val> <= %u. Using standard value %lu now.",
				1, INT_MAX, param_queuelength);
		}
	}
	if (opt_w) {
		if (sscanf(str_w, "%lu", &param_packettimeout) != 1
		||	(param_packettimeout < 1 || param_packettimeout > INT_MAX)) {
			param_packettimeout = 1000;
			tst_resm(TWARN,
				"Bad timeout to wait for a packet. Must be %u <= <val> <= %u. Using standard value %lu now.",
				1, INT_MAX, param_packettimeout);
		}
	}
	if (opt_W) {
		if (sscanf(str_W, "%lu", &param_packetdelay) != 1
		||	(param_packetdelay > INT_MAX)) {
			param_packetdelay = 0;
			tst_resm(TWARN,
				"Bad time to wait between sending packets. Must be %u <= <val> <= %u. Using standard value %lu now.",
				0, INT_MAX, param_packetdelay);
		}
	}
	if (opt_s) {
		if (sscanf(str_s, "%lu", &param_statinterval) != 1
		||	(param_statinterval > INT_MAX)) {
			param_statinterval = 2000;
			tst_resm(TWARN,
				"Bad interval for statistics. Must be %u <= <val> <= %u. Using standard value %lu now.",
				0, INT_MAX, param_statinterval);
		}
	}
	if (opt_r) {
		if (sscanf(str_r, "%lu", &param_errorrate) != 1
		||	(param_errorrate > 1000)) {
			param_errorrate = 20;
			tst_resm(TWARN,
				"Bad maximum error-rate considered okay. Must be %u <= <val> <= %u. Using standard value %lu now.",
				0, 1000, param_errorrate);
		}
	}
	if (opt_k) {
		if (sscanf(str_k, "%lu", &param_maxfragments) != 1
		||	(param_maxfragments < 1 || param_maxfragments > MAX_PACKET_SIZE)) {
			param_maxfragments = 1;
			tst_resm(TWARN,
				"Bad maximum number of data-chunks (fragments). Must be %u <= <val> <= %u. Using standard value %lu now.",
				1, MAX_PACKET_SIZE, param_maxfragments);
		}
	}
	if (opt_u) {
		param_protocol = USE_UDP_PROTOCOL;
	}
	if (opt_a) {
		param_protocol = USE_DEFAULT_PROTOCOL;
		tst_resm(TWARN,
			"TCP protocol is not yet supported on this platform. Using default protocol (UDP).");
	}
	if (opt_b) {
		param_protocol = USE_DEFAULT_PROTOCOL;
		tst_resm(TWARN,
			"ICMP protocol is not yet supported on this platform. Using default protocol (UDP).");
	}
	
	if (!opt_g || !str_g)
		tst_brkm(TBROK, cleanup,
			"No target address specified.");
	
	if(opt_v) {
		param_verbosity = 2;
	}
	
	if(opt_q) {
		param_verbosity = 0;
	}
	
	/* Parameter cleaning*/
	if (param_maxfragments > param_packetsize)
		param_maxfragments = param_packetsize;

	if (param_protocol == USE_DEFAULT_PROTOCOL)
		param_protocol = USE_UDP_PROTOCOL;

	memset(&addrHints, 0, sizeof(struct addrinfo)); 	// make sure the struct is empty

	addrHints.ai_family = AF_INET;     			// don't care IPv4 or IPv6
	switch (param_protocol) {
		case USE_UDP_PROTOCOL :
			addrHints.ai_socktype = SOCK_DGRAM;
			addrHints.ai_protocol = IPPROTO_UDP;
			break;

		case USE_TCP_PROTOCOL:
			addrHints.ai_socktype = SOCK_STREAM;
			addrHints.ai_protocol = IPPROTO_TCP;
			break;

		case USE_ICMP_PROTOCOL:
			addrHints.ai_socktype = SOCK_RAW;
			addrHints.ai_protocol = IPPROTO_ICMP;
			break;
	}
	
	param_szTarget = str_g;
	if ((i = getaddrinfo(param_szTarget, "7", &addrHints, &addrInfo)) != 0)
		tst_brkm(TBROK, cleanup,
			"getaddrinfo error: %s",
			gai_strerror(i));

	/* Setup */
	if (setup() != 0){
		exit(2);
	}

	// Tell user about what we're going to do.
	switch (param_protocol) {
		case USE_UDP_PROTOCOL: 
			protocol_name = "UDP";
			break;
		case USE_TCP_PROTOCOL: 
			protocol_name = "TCP";
			break;
		case USE_ICMP_PROTOCOL: 
			protocol_name = "ICMP";
			break;
	}

	out(1, "Starting iptest to Host %hs (%s protocol) with %u packets of size %u (max %u in parallel).",
		inet_ntoa(((struct sockaddr_in*)addrInfo->ai_addr)->sin_addr), protocol_name,
		param_numpackets, param_packetsize, param_queuelength);
	out(1, "Packets will time out after %u ms. Allowed error rate is max %u/1000",
		param_packettimeout, param_errorrate);

	
	/* Create Threads */
	r = pthread_create(&workerThreads[0], NULL, &InitThread, NULL);
	r |= pthread_create(&workerThreads[1], NULL, &RecvThread, NULL);
	r |= pthread_create(&workerThreads[2], NULL, &SendThread, NULL);

	r |= pthread_create(&statThread, NULL, &StatThread, (void*)&ulErrorRate);

	if (r != 0) {
		tst_brkm(TBROK, cleanup, "Failed to create worker threads!");
		exit(2);
	}

	/* Wait for threads to finish */
	for(i = 0; i<3; i++) {
		r = pthread_join(workerThreads[i], NULL); 
		if (r != 0) {
			tst_brkm(TBROK, cleanup, "ERROR; return code from pthread_join() is %d", r);
			exit(2);
		}
	}
	
	/* Tell statistics thread that we're done */
	pthread_mutex_lock(&workersDoneMutex);
	pthread_cond_signal(&workersDoneCond);
	pthread_mutex_unlock(&workersDoneMutex);

	/* Wait for it to finish statistics output and calculate overall error rate */
	pthread_join(statThread, NULL);

	if (ulErrorRate > param_errorrate) {
		tst_resm(TFAIL, "Too many packages got lost, or where invalid. Error rate was %lu%% - Test failed!", ulErrorRate/10);
		r = 1;
	}
	else {
		tst_resm(TPASS, "Error rate was <%lu.%lu%% - Test passed!", ulErrorRate/10, ulErrorRate%10);
		r = 0;
	}
	
	cleanup();
	exit(r);
}
