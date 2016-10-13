/******************************************************************************/
/* Copyright (C) 2010 Garz&Fricke GmbH						 			      */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        rs485pingpong.c                                               */
/*                                                                            */
/* Description: This is a test for the RS485 half duplex mode.                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   rs485pingpong                                                 */
/*                                                                            */
/* Test Assertion                                                             */
/* & Strategy:  A test string is sent on the serial port. The opposite side   */
/*              is supposed to echo the string back to the test. The test     */
/*              will echo the received string back again. This is repeated    */
/*              for a specified number of times. Thus, the string gets "ping- */
/*              ponged". Finally, the last received string is compared to the */
/*              initially sent string. If they are equal the test is passed,  */
/*              else it fails.                                                */
/*              The opposite side might be a simple Windows PC running the    */
/*              RS485Test (http://gfweb/svn/sw/som/neso/trunk/Tests/          */
/*                         RS485PingPong/RS485TestWindowsApp/)                */
/*              with the "Echo on" option activated. The PC has to be         */
/*              connected to the device via an RS232-RS485 converter.         */
/*              Alternativly, it might be another device running this test.   */
/*                                                                            */
/* Author:      Tim Jaacks <tim.jaacks@garz-fricke.com                        */
/*                                                                            */
/******************************************************************************/

/* Standard Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <linux/serial.h>
#include <asm/ioctls.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

/* Local Defines */
#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif

/* Extern Global Variables */
extern int  Tst_count;               /* counter for tst_xxx routines.         */
extern char *TESTDIR;                /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID     = "rs485pingpong  ";  /* test program identifier.              */
int  TST_TOTAL = 1;                  /* total number of tests in this file.   */

int  fd;                             /* Handle for open serial port           */
char* device_name = "/dev/ttymxc2";  /* String containing the device name     */
struct termios termios;              /* termios structure for serial port     */
int bytes = 8;                       /* Number of bytes to be sent            */
                                     /*     Note: The echoing end has to be   */
                                     /*           set to the same number.     */
char* test_string;                   /* String to be sent                     */
char* received_string;               /* Received string                       */
int repetitions = 1;                 /* Number of repetitions                 */
int timeout = 5;                     /* Receive timeout in seconds            */
int baudrate = 9600;                 /* Baud rate in bps                      */


speed_t baud_constant[] = { B50, B75, B110, B134, B150, B200, B300, B600,
							B1200, B1800, B2400, B4800, B9600, B19200, B38400,
							B57600, B115200, B230400, B460800, B500000,
							B576000, B921600, B1000000, B1152000, B1500000,
							B2000000, B2500000, B3000000, B3500000
						   };

unsigned long baud_value[] = {  50, 75, 110, 134, 150, 200, 300, 600,
								1200, 1800, 2400, 4800, 9600, 19200, 38400,
								57600, 115200, 230400, 460800, 500000,
								576000, 921600, 1000000, 1152000, 1500000,
								2000000, 2500000, 3000000, 3500000
							 };

/* Extern Global Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    cleanup                                                       */
/*                                                                            */
/* Description: Performs all one time clean up for this test on successful    */
/*              completion,  premature exit or  failure. Closes all temporary */
/*              files, removes all temporary directories exits the test with  */
/*              appropriate return code by calling tst_exit() function.       */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*              On success - Exits calling tst_exit(). With '0' return code.  */
/*                                                                            */
/******************************************************************************/
extern void
cleanup()
{
	/* Free all allocated memory. */

	if (test_string)
		free(test_string);
	if (received_string)
		free(received_string);
	
	/* Close all open file descriptors. */

	if (close(fd) == -1)
	{
		tst_resm(TWARN, "close(%s) Failed, errno=%d : %s",
		device_name, errno, strerror(errno));
	}

	/* Remove all temporary directories used by this test. */

	// Insert real code here 

	/* kill child processes if any. */

	// Insert code here

	/* Exit with appropriate return code. */

	tst_exit();
}


/* Local  Functions */
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
void
setup()
{
	int i;
	struct serial_rs485 rs485;
	speed_t baud = B0;
	
	/* Check if baudrate is supported */
	for (i = 0; i < ARRAY_SIZE(baud_constant); i++)
	{
		if (baud_value[i] == baudrate)
		{
			baud = baud_constant[i];
			break;
		}
	}
	if (baud == B0)
		tst_brkm(TBROK, cleanup, "Baud rate not supported (%d)", baudrate);

	/* Create test string */
	
	// Allocate space for send string
	test_string = malloc(sizeof(char) * (bytes + 1));
	
	// Fill buffer with alphabet
	test_string[0] = 'A';
	for (i=1; i<bytes; i++)
	{
		test_string[i] = test_string[i-1] + 1;
		if (test_string[i] > 'Z')
			test_string[i] = 'A';
	}
	// Last byte is 0 to be able to treat buffer as a string
	test_string[bytes] = 0;

	// Allocate space for received string
	received_string = malloc(sizeof(char) * (bytes + 1));
	received_string[bytes] = 0;

	
	/* Initialize serial communication */

	// Open port
    fd = open (device_name, O_RDWR | O_SYNC);
    if (fd == -1) {
		tst_brkm(TBROK, cleanup,
			"Unable to open serial port %s.  Error:%d, %s\n",
			device_name, errno, strerror(errno));
    }
	
	// Enable RS485 half duplex mode
	rs485.flags = SER_RS485_ENABLED | SER_RS485_RTS_ON_SEND;
	if (ioctl(fd, TIOCSRS485, &rs485) != 0)
	{
		tst_brkm(TBROK, cleanup,
			"Unable to set RS485 half duplex mode on serial port %s.  Error:%d, %s\n",
			device_name, errno, strerror(errno));
	}
	
    // Get old attribute
    if (tcgetattr (fd, &termios)) {
		// This is not a serial device
		tst_brkm(TBROK, cleanup,
			"Can't read device configuration. Is this a serial device? \
			Error:%d, %s\n", errno, strerror(errno));
    }
    
    // Set Speed into termios structure
    if (cfsetospeed (&termios, baud)) {
		tst_brkm(TBROK, cleanup,
			"Can't set baud rate.  Error:%d, %s\n",
			errno, strerror(errno));
    }
	
    // Disable Echo, make Data=8, Stop=7, no parity, disable all controls
    // see "man cfmakeraw"
    cfmakeraw (&termios);

	// Set internal timeout value to 10 * 0.1 seconds, i.e. read() will
	// always return after 1 second, even if there is no character present.
	termios.c_cc[VMIN] = 0;
	termios.c_cc[VTIME] = 10;

    // set new attribute
    if (tcsetattr (fd, TCSANOW, &termios)) {
		tst_brkm(TBROK, cleanup,
			"Can't set device configuration.  Error:%d, %s\n",
			errno, strerror(errno));
    }

	return;
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
void help()
{
	printf("  -d s    Select serial port specified by device name s (default: /dev/ttymxc2)\n");
	printf("  -s x    Set baudrate to x bps (default: 9600)\n");
	printf("  -b x    Send x bytes (default: 8)\n");
	printf("  -r x    Repeat send and receive x times (default: 1)\n");
	printf("  -o x    Set receive timeout to x seconds (default: 5)\n");
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
/*               -l - Number of iteration                                     */
/*               -v - Prints verbose output                                   */
/*               -V - Prints the version number                               */
/*                                                                            */
/* Exit:       On failure - Exits by calling cleanup().                       */
/*             On success - exits with 0 exit value.                          */
/*                                                                            */
/******************************************************************************/
int
main(int   argc,    /* number of command line parameters                      */
	 char **argv)   /* pointer to the array of the command line parameters.   */
{
	int cycle_count = 0;
	int byte_count = 0;

	int opt_device_name = 0, opt_repetitions = 0, opt_bytes = 0,
		opt_timeout = 0, opt_baudrate = 0;
	char *str_device_name, *str_repetitions, *str_bytes, *str_timeout,
		*str_baudrate, *msg;
	
	option_t options[] = {
		{"d:", &opt_device_name, &str_device_name},
		{"r:", &opt_repetitions, &str_repetitions},
		{"b:", &opt_bytes, &str_bytes},
		{"o:", &opt_timeout, &str_timeout},
		{"s:", &opt_baudrate, &str_baudrate},
		{NULL, NULL, NULL}
	};
	
	time_t t;
	int timeout_happened = FALSE;
	

	/* Parse options. */

	if ((msg=parse_opts(argc, argv, options, &help)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	if (opt_device_name)
		device_name = str_device_name;
	if (opt_repetitions)
		repetitions = atoi(str_repetitions);
	if (opt_bytes)
		bytes = atoi(str_bytes);
	if (opt_timeout)
		timeout = atoi(str_timeout);
	if (opt_baudrate)
		baudrate = atoi(str_baudrate);


	/* Perform global test setup, call setup() function. */
	setup();


	/* Print test Assertion using tst_resm() function with argument TINFO. */
	tst_resm(TINFO, "Starting RS485 ping pong test.");
	tst_resm(TINFO, "Sending string \"%s\" to %s (%d,8N1) and waiting for echo.",
             test_string, device_name, baudrate);
	tst_resm(TINFO, "(repetitions: %d, timeout: %d seconds)", repetitions, timeout,
             test_string, device_name);


	/* Test Case Body. */

	// Send test string
	write(fd, test_string, bytes);
	
	// Wait for echo and send received string back repeatedly
	while (cycle_count < repetitions)
	{
		// Get current time for measuring timeout
		t = time(NULL);
		while (byte_count < bytes && !timeout_happened)
		{
			// Read serial port
			byte_count += read(fd, &received_string[byte_count], 1);
			// Check if timeout occured
			if (time(NULL) - t > timeout)
				timeout_happened = TRUE;
		}
		if (timeout_happened)
			break;
		// Write received string back
		write(fd, received_string, bytes);
		byte_count = 0;
		cycle_count++;
	}

	
	/* Print results and exit test-case */

	if (!timeout_happened)
	{
		tst_resm(TINFO, "Last received string \"%s\"",
				 received_string);
		if (strcmp(test_string, received_string) == 0)
			tst_resm(TPASS, "Received string equals sent string. Test passed.");
		else
			tst_resm(TFAIL, "Received string does not equal sent string. Test failed.");
	}
	else
		tst_resm(TFAIL, "Timeout occured. Other side did not respond.");
	
	cleanup();
	tst_exit();
}