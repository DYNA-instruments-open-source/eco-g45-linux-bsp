/******************************************************************************/
/* Copyright (C) 2011 Garz&Fricke GmbH						 			      */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        serial.c                                                      */
/*                                                                            */
/* Description: This is a test for the UART.                                 */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   serial                                                        */
/*                                                                            */
/* Test Assertion                                                             */
/* & Strategy:  A random test string is isent in the serial Port. The         */
/*              opposite side is supposed to echo the string back to the test.*/
/*              The received version is compared to the data which was sent.  */
/*              This is repeated a number of times. When all messages where   */
/*              equal, the test is passed, else it fails.                     */
/*              The opposite side might be a simple Windows PC running the    */
/*              serlive tool (http://gufweb/svn/sw/som/CE-Tools/trunk/        */
/*              serlive/serlive_c/)                                           */
/*              with the "Echo" option activated. Alternatively the Rx and Tx */
/*              lines of the port under test can be crossed, so that all data */
/*              gets looped back.                                             */
/*                                                                            */
/* Author:      Hendrik Woidt <hendrik.woidt@garz-fricke.com                  */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>

#include <pthread.h>
#include <termios.h>
#include <fcntl.h>


#include "test.h"
#include "usctest.h"


#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif



#define STATE_EXIT				0
#define STATE_SENDING			1
#define STATE_RECEIVING			2
#define STATE_VALIDATE			3
#define STATE_STARTNEXT			4
#define STATE_POPULATEBUFFER	5
#define STATE_TIMEOUT			6


/* default values: */
char *tty_dev = "/dev/ttymxc1";
speed_t setting_baud_rate = B115200;
int setting_count = 10;
int setting_size = 32;
int setting_timeout = 1000;
int setting_flow_control = 0;
int setting_verbose = 1;

/* global variables */
char *TCID     = "serial         ";	/* test program identifier.              */
int  TST_TOTAL = 1;					/* total number of tests in this file.   */

int tty_fd;
struct termios *termios;		/* Holds the structure with port settings */

char *send_buffer;
char *receive_buffer;

struct timeval *time_tst_start, *time_begin, *time_sent, *time_received;

/* global result variables */
int packages_sent;
int packages_received;

int result_num_pass;
int result_num_timeouts;
int result_num_fail;
double result_package_duration;

int state;						/* global state variable to synchronize the Rx/Tx thread */

int test_brk = 0;				/*	global variable to indicate that there is an unexpected 
									failure when terminating*/

void setup();
void cleanup(int return_code);
void terminate();

/* Helper functions: */
void fill_transmit_buffer(char *buffer, int size) {
	int i;
	for (i = 0; i < size; i++) {
		buffer[i] = (char) rand();
		/* avoid control characters (and the % to prevent serlive from 
		   dying on this when displaying the echoed characters */
		if(buffer[i] < 32 || buffer[i] > 126 || buffer[i] == '%') {
			i--;
		}
	}
}

int validate_buffers(char *buffer1, char* buffer2, int size) {
	int i;
	for(i = 0; i < size; i++) {
		if(buffer1[i] != buffer2[i]) {
			return 0;
		}
	}
	return 1;
}

speed_t parse_baud(char *str_baud_rate) {
	int baud = atoi(str_baud_rate);
	speed_t ret = 0; 
	
	switch(baud) {
		case 50:
			ret = B50;
			break;
		case 75:
			ret = B50;
			break;
		case 110:
			ret = B110;
			break;
		case 134:
			ret = B134;
			break;
		case 150:
			ret = B150;
			break;
		case 200:
			ret = B200;
			break;
		case 300:
			ret = B300;
			break;
		case 600:
			ret = B600;
			break;
		case 1200:
			ret = B1200;
			break;
		case 1800:
			ret = B1800;
			break;
		case 2400:
			ret = B2400;
			break;
		case 4800:
			ret = B4800;
			break;
		case 9600:
			ret = B9600;
			break;
		case 19200:
			ret = B19200;
			break;
		case 38400:
			ret = B38400;
			break;
		case 57600:
			ret = B57600;
			break;
		case 115200:
			ret = B115200;
			break;
		case 230400:
			ret = B230400;
			break;
		default:
			test_brk = TRUE;
			tst_brkm(TBROK, terminate, "Bad baudrate (%i). use one of the following: 50, 75, 110, 134, 150, 200, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400", baud);
			break;
	}
	
	return ret;
}

int time_ms_diff(struct timeval *tv1, struct timeval *tv2) {
	return (tv2->tv_sec - tv1->tv_sec)*1000 + (tv2->tv_usec - tv1->tv_usec)/1000;
}

int chk_exit() {
	// return whether the test is complete or not
	if(packages_sent == setting_count)
		return 1;
	else
		return 0;
}

void save_result(int is_valid, int is_timeout, int bytes_received) {
	char *passfail = "fail";
	char *timeout = "=> Timeout!";

	result_package_duration += time_ms_diff(time_begin, time_received);
	
	if(is_timeout) {
		result_num_timeouts++;
		result_num_fail++;
	}
	else {
		timeout = "";
		if(is_valid) {
			// pass
			result_num_pass++;
			passfail = "pass";
		}
		else {
			// invalid data => fail
			result_num_fail++;
		}
	}
	
	if(setting_verbose >= 2) {
		tst_resm(TINFO, "Package #%i: received %i bytes within %ims - %s %s", packages_sent, bytes_received, time_ms_diff(time_begin, time_received), passfail, timeout);
	}
}


/******************************************************************************/
/*                                                                            */
/* Function:    cleanup                                                       */
/*                                                                            */
/* Description: Performs all one time clean up for this test on successful    */
/*              completion,  premature exit or  failure. Closes all temporary */
/*              files, removes all temporary directories exits the test with  */
/*              appropriate return code.                                      */
/*                                                                            */
/* Input:       Return code to exit with.                                     */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      Exits with the specified return code.                         */
/*                                                                            */
/******************************************************************************/
void cleanup(int ret) {
	if(setting_verbose >= 3) fprintf(stderr, "doing cleanup");

	if (close(tty_fd) == -1)
	{
		tst_resm(TWARN, "close(%s) Failed, errno=%d : %s", 
			tty_dev, errno, strerror(errno));
	}
	if(send_buffer)
		free(send_buffer);
	if(receive_buffer)
		free(receive_buffer);
	if(termios)
		free(termios);
	
	exit(ret);
}


/******************************************************************************/
/*                                                                            */
/* Function:    terminate                                                     */
/*                                                                            */
/* Description: Check whether the test has been passed or not and print a     */
/*              test summary.                                                 */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      Test summary according to verboseness.                        */
/*                                                                            */
/* Return:      On failure - Exits calling cleanup(). With '1' return code.   */
/*              On success - Exits calling cleanup(). With '0' return code.   */
/*                                                                            */
/******************************************************************************/
void terminate() {
	int return_code;
	
	if(test_brk == TRUE) {
		return_code = 2;
	}
	else if(result_num_fail == 0) {
		tst_resm(TPASS, "Sending and receiving of %i packages of size %i bytes was successful. Test passed.", setting_count, setting_size);
		return_code = 0;
	}
	else {
		tst_resm(TFAIL, "%i out of %i Packages where invalid. Test failed.", result_num_fail, setting_count);
		return_code = 1;
	}

	if(setting_verbose >= 1) {
		if(result_num_timeouts != setting_count)
			tst_resm(TINFO, "Average package duration: %.2fms => %.2f bytes/sec", (result_package_duration / packages_sent), ((packages_sent * setting_size) / result_package_duration)*1000);
		gettimeofday(time_received, NULL);
		tst_resm(TINFO, "Test duration: %.2f seconds", (double)time_ms_diff(time_tst_start, time_received) / 1000);
	}
	
	cleanup(return_code);
}

/******************************************************************************/
/*                                                                            */
/* Function:    setup                                                         */
/*                                                                            */
/* Description: Performs all one time setup for this test. This function is   */
/*              typically used to capture signals, create temporary dirs      */
/*              and temporary files that may be used in the course of this    */
/*              test.                                                         */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits by calling cleanup().                      */
/*              On success - returns 0.                                       */
/*                                                                            */
/******************************************************************************/
void setup() {
	
	/* allocate momory */
	if((receive_buffer = malloc(sizeof(char) * setting_size)) == 0) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate,
			"Unable to allocate Memory.  Error:%d, %s\n",
			errno, strerror(errno));
	};
	if((send_buffer = malloc(sizeof(char) * setting_size)) == 0) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate,
			"Unable to allocate Memory.  Error:%d, %s\n",
			errno, strerror(errno));
	}
	if((termios = malloc(sizeof(struct termios))) == 0) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate,
			"Unable to allocate Memory.  Error:%d, %s\n",
			errno, strerror(errno));
	}
	if((time_tst_start = malloc(sizeof(struct timeval) * 4)) == 0) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate,
			"Unable to allocate Memory.  Error:%d, %s\n",
			errno, strerror(errno));		
	}
	else {
		time_begin = time_tst_start + sizeof(struct timeval);
		time_sent = time_begin + sizeof(struct timeval);
		time_received = time_sent + sizeof(struct timeval);
	}
	/* seed the pseudo random number generator for testdata generation */
	gettimeofday(time_sent, NULL);
	srand(time_sent->tv_sec);
	
	/* open and configure serial port */
	tty_fd = open(tty_dev, O_RDWR | O_NONBLOCK);
	
	if(tty_fd == -1) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate,
			"Unable to open serial port %s.  Error:%d, %s\n",
			tty_dev, errno, strerror(errno));
	}
	
	if(tcgetattr(tty_fd, termios)) {
		// This is not a serial device
		test_brk = TRUE;
		tst_brkm(TBROK, terminate,
			"Can't read device configuration. Is this a serial device? \
			Error:%d, %s\n", errno, strerror(errno));
	}
	
	// Set Speed into termios structure
	if (cfsetospeed (termios, setting_baud_rate)) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate,
			"Can't set baud rate.  Error:%d, %s\n",
			errno, strerror(errno));
	}
	
	if (cfsetispeed (termios, setting_baud_rate)) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate,
			"Can't set baud rate.  Error:%d, %s\n",
			errno, strerror(errno));
	}
	
	// Disable Echo, make Data=8, Stop=7, no parity, disable all controls
	// see "man cfmakeraw"
	cfmakeraw(termios);

	// Set internal timeout value to 10 * 0.1 seconds, i.e. read() will
	// always return after 1 second, even if there is no character present.
	//termios->c_cc[VMIN] = 0;
	//termios->c_cc[VTIME] = 0;
	if(setting_flow_control == 1) {
		termios->c_iflag |= IXON | IXOFF;
	}

	// set new attribute
	if (tcsetattr(tty_fd, TCSANOW, termios)) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate,
			"Can't set device configuration.  Error:%d, %s\n",
			errno, strerror(errno));
	}

}

/* Thread to read from the serial port. This thread also controls the test flow */
void *receive_thread(void *ptr) {
	// do receiving and timeout checks here
	int bytes_received = 0;
	int bytes_read = 0;
	
	while(state) {
		switch(state) {
			case STATE_STARTNEXT:
				bytes_received = 0;
				bytes_read = 0;

				if(chk_exit()) {
					if(setting_verbose >= 3) fprintf(stderr, "terminating\n");
					
					state = STATE_EXIT;
				}
				else {
					if(setting_verbose >= 3) fprintf(stderr, "starting next transmission\n");
					
					state = STATE_POPULATEBUFFER;
				}
				break;
			case STATE_RECEIVING:
				gettimeofday(time_received, NULL);
				if(time_ms_diff(time_begin, time_received) > setting_timeout) {
				
					state = STATE_TIMEOUT;
					break;
				}
				if(bytes_received == setting_size) {
				
					state = STATE_VALIDATE;
					break;
				}
				/* Fallthrough! */
			case STATE_SENDING:
				bytes_read = read(tty_fd, receive_buffer + bytes_received, setting_size - bytes_received);
				if(bytes_read == -1) {
					if(errno != EAGAIN && errno != EWOULDBLOCK) {
						// ERROR!
						test_brk = TRUE;
						tst_brkm(TBROK, terminate, "Could not read bytes from input: %i %s", errno, strerror(errno));
					}
				}
				else {
					bytes_received += bytes_read;
					gettimeofday(time_received, NULL);
				}
				break;
			case STATE_TIMEOUT:
				if(setting_verbose >= 3) fprintf(stderr, "timeout! received %i bytes\n", bytes_received);
				save_result(FALSE, TRUE, bytes_received);
				
				state = STATE_STARTNEXT;
				break;
			case STATE_VALIDATE:
				if(setting_verbose >= 3) fprintf(stderr, "validating received data\n");
				save_result(validate_buffers(send_buffer, receive_buffer, setting_size), FALSE, bytes_received);
				
				state = STATE_STARTNEXT;
				break;
			default:
				// sleep some short time
				usleep(100);
				break;
		}
	}
	
	return NULL;
}

/* Function to perform the test by writing random Data to the serial Port. 
   Meant to be executed in parallel to receive_thread() */
void transmit() {
	int bytes_written = 0;
	int bytes_sent = 0;
	
	while(state) {
		switch(state) {
			case STATE_POPULATEBUFFER:
				if(setting_verbose >= 3) fprintf(stderr, "populating buffer\n");
				
				bytes_written = 0;
				bytes_sent = 0;
				
				fill_transmit_buffer(send_buffer, setting_size);

				tcflush(tty_fd, TCIOFLUSH);

				gettimeofday(time_begin, NULL);
				
				state = STATE_SENDING;
				/* fall through */
			case STATE_SENDING:
				gettimeofday(time_sent, NULL);
				if(time_ms_diff(time_begin, time_sent) > setting_timeout) {
					state = STATE_TIMEOUT;
					break;
				}
				bytes_written = write(tty_fd, send_buffer + bytes_sent, setting_size - bytes_sent);
				
				if(bytes_written == -1) {
					// ERROR!
					if(errno != EAGAIN && errno != EWOULDBLOCK) {
						test_brk = TRUE;
						tst_brkm(TBROK, terminate, "Could not write bytes to output: %i %s", errno, strerror(errno));
					}
				}
				else {
					bytes_sent += bytes_written;
					if(bytes_sent == setting_size) {
						// package was sent completely
						if(setting_verbose >= 3) fprintf(stderr, "package transmitted\n");
						gettimeofday(time_sent, NULL);
						packages_sent++;
						
						state = STATE_RECEIVING;
					}
				}
				break;
			default:
				usleep(100);
				// sleep some short time
				break;
		}
	}
}


/******************************************************************************/
/*                                                                            */
/* Function:    help                                                          */
/*                                                                            */
/* Description: This function is called when the test is started with         */
/*              parameter -h. It displays all parameter options specifically  */
/*              available for this test.                                      */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      Nothing.                                                      */
/*                                                                            */
/******************************************************************************/
void help() {
	printf(" Test programm for the serial port: A random string is sent and expected to be echoed back\n");
	printf("  -d s    Select serial port specified by device name s (default: /dev/ttymxc1)\n");
	printf("  -b s    Set Baud rate to x (default: 115200)\n");
	printf("  -c x    Repeat send and receive x times (default: 10)\n");
	printf("  -s x    Send x bytes per package (default: 32)\n");
	printf("  -o x    Set receive timeout to x milliseconds (default: 1000)\n");
	printf("  -fn     turn off flow control (default)\n");
	printf("  -fs     use software flow control (XON/XOFF)\n");
	printf("  -v      Be verbose. This will print out a status message for each package which is sent or received\n");
	printf("  -vv     Be really verbose. This will print out debug messages on stderr to trace what the programm currently does\n");
	printf("  -q      Be quiet. Only the test's pass/fail message is printed\n");
	printf("  -h      display this help message\n");
}

/******************************************************************************/
/*                                                                            */
/* Function:    main                                                          */
/*                                                                            */
/* Description: Entry point to this test-case. It parses all the command line */
/*              inputs, calls the global setup and executes the test. It logs */
/*              the test status and results appropriately using the LTP API's */
/*              On successful completion or premature failure, cleanup() func */
/*              is called and test exits with an appropriate return code.     */
/*                                                                            */
/* Input:       Describe input arguments to this test-case                    */
/*               -d - Serial port device file name                            */
/*               -b - Baud rate                                               */
/*               -c - Number of iteration                                     */
/*               -s - Size of data chunks                                     */
/*               -o - Timeout after which to stop waiting for complete message*/
/*               -fs - Enable software flow control (XON XOFF)                */
/*               -fn - Disable flow control                                   */
/*               -v - Be verbose. This will print out a status message for    */
/*                    each package which is sent or received                  */
/*               -vv - Be really verbose. This will print out debug messages  */
/*                     on stderr to trace what the programm currently does    */
/*               -q - Be quiet. Only the test's pass/fail message is printed  */
/*                                                                            */
/* Exit:       On error   - exits with 2 exit value.                          */
/*             On failure - exits with 1 exit value.                          */
/*             On success - exits with 0 exit value.                          */
/*                                                                            */
/******************************************************************************/
int main(int argc, char **argv) {
	pthread_t thread_rx;
	int opt;
	int baud = 0;
	
	/* parse commandline options: */
	while ((opt = getopt(argc, argv, "vqhd:b:c:s:o:f:")) != -1) {
		switch (opt) {
			case 'd':
				tty_dev = optarg;
				break;
			case 'b':
				baud = atoi(optarg);
				setting_baud_rate = parse_baud(optarg);
				break;
			case 'c':
				setting_count = atoi(optarg);
				break;
			case 's':
				setting_size = atoi(optarg);
				break;
			case 'o':
				setting_timeout = atoi(optarg);
				break;
			case 'f':
				if(optarg[0] == 'n')
					setting_flow_control = 0;
				else if(optarg[0] == 's')
					setting_flow_control = 1;
				else
					setting_flow_control = 0;
				break;
			case 'h':
				help();
				cleanup(1);
				break;
			case 'v':
				setting_verbose++;
				break;
			case 'q':
				setting_verbose--;
				break;
			default:
				help();
				cleanup(1);
				break;
		}
	}

	if(setting_verbose >= 3)
		fprintf(stderr, "option parsing done, doing setup\n");
	
	/* allocate memory, open port etc. */
	setup();
	
	state = STATE_STARTNEXT;
	
	if(setting_verbose >= 1)
		tst_resm(TINFO, "Starting serial Test on %s with %i baud. %i packages of %i bytes will be transmitted.", tty_dev, baud ? baud : 115200, setting_count, setting_size);
	
	/* save timestamp to compute overall test duration */
	gettimeofday(time_tst_start, NULL);
	
	if(setting_verbose >= 3)
		fprintf(stderr, "setup complete, starten receiver thread\n");
	
	/* start control/receiver thread */
	pthread_create(&thread_rx, NULL, receive_thread, NULL);
	
	if(setting_verbose >= 3)
		fprintf(stderr, "thread started, beginnig package tranceive\n");

	/* start transmitting packages: */
	transmit();
	
	if(setting_verbose >= 3)
		fprintf(stderr, "sending complete, waiting for thread to terminate\n");
	
	/* wait for receiver thread to close, after performing the test */
	pthread_join(thread_rx, NULL);
	
	/* print test summary and exit */
	terminate();
	
	if(setting_verbose >= 3)
		fprintf(stderr, "terminate did not exit... somthing is really wrong!\n");
	
	return 2;
}
