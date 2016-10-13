/******************************************************************************/
/* Copyright (C) 2010 Garz&Fricke GmbH						 			      */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        qwheel.c                                                      */
/*                                                                            */
/* Description: This is a test for the QWheel.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   qwheel                                                        */
/*                                                                            */
/* Test Assertion                                                             */
/* & Strategy:  On start of the test, all LEDs are turned on to see whether   */
/*              they are working. The user has to press the middle key of the */
/*              wheel within a certain amount of time. After that, he has to  */
/*              subsequently touch the wheel at four different positions,     */
/*              which are specified by a message on the console and by an     */
/*              according LED. If all the five touches are recognized, the    */
/*              test is passed. If a timeout occurs, the test fails.          */
/*                                                                            */
/* Author:      Tim Jaacks <tim.jaacks@garz-fricke.com>                       */
/*                                                                            */
/******************************************************************************/

/* Standard Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <asm/ioctls.h>
#include <linux/input.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

/* Local Defines */
#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif

#define MAX_WHEEL_POS 255

/* Extern Global Variables */
extern int  Tst_count;               /* counter for tst_xxx routines.         */
extern char *TESTDIR;                /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID     = "qwheel         ";  /* test program identifier.              */
int  TST_TOTAL = 1;                  /* total number of tests in this file.   */

int fd_qwheel = -1;                  /* File descriptor for QWheel            */
int fd_sysfs_gpo_leds = -1;          /* File descriptor for LEDs              */

char* qwheel_dev = "/dev/event0";    /* String containing the device name     */
char* sysfs_dev = "/sys/class/input/event0/device/gpo";

int timeout = 15;                    /* Key-press timeout in seconds          */


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

	/* Close all open file descriptors. */

	if (close(fd_qwheel) == -1)
	{
		tst_resm(TWARN, "close(%s) Failed, errno=%d : %s",
		qwheel_dev, errno, strerror(errno));
	}

	if (close(fd_sysfs_gpo_leds) == -1)
	{
		tst_resm(TWARN, "close(%s) Failed, errno=%d : %s",
		sysfs_dev, errno, strerror(errno));
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
	// Open the QWheel driver
    printf ("Open port ... \n"); fflush (stdout);
    fd_qwheel = open (qwheel_dev, O_RDONLY | O_NONBLOCK);
    if (fd_qwheel < 0) {
		tst_brkm(TBROK, cleanup,
			"Unable to open QWheel driver %s.  Error:%d, %s\n",
			qwheel_dev, errno, strerror(errno));
    }

	// Open the sysfs entry for the gpo leds */
	fd_sysfs_gpo_leds = open(sysfs_dev, O_RDWR);
	if(fd_sysfs_gpo_leds < 0) {
		tst_brkm(TBROK, cleanup,
			"Unable to open sysfs driver %s.  Error:%d, %s\n",
			sysfs_dev, errno, strerror(errno));
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
	printf("  -d s    Select QWheel device name s (default: /dev/event0)\n");
	printf("  -s s    Select sysfs device name s (default: /sys/class/input/event0/device/gpo\n");
	printf("  -o x    Set keypress timeout to x seconds (default: 15)\r\n");
}


int wait_for_key_pressed(int timeout)
{
	time_t t;
	int ret;
	struct input_event ev;

	// First, flush the input buffer
	while (read(fd_qwheel, &ev, sizeof(struct input_event)) > 0);

	// Get current time for measuring timeout
	t = time(NULL);
	
	while (time(NULL) - t < timeout)
	{
		ret = read(fd_qwheel, &ev, sizeof(struct input_event));
		if (ret > 0)
		{
			if (ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0)
			{
				return MAX_WHEEL_POS + 1;
			}
			else if (ev.type == EV_ABS && ev.code == ABS_X)
			{
				return ev.value;
			}
		}
	}
	
	return -1;
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

	struct input_event ev;
	int ret = 0;
	char leds[3];

	int opt_qwheel_dev = 0, opt_sysfs_dev = 0, opt_timeout = 0;
	char *str_qwheel_dev, *str_sysfs_dev, *str_timeout, *msg;
	
	option_t options[] = {
		{"d:", &opt_qwheel_dev, &str_qwheel_dev},
		{"s:", &opt_sysfs_dev, &str_sysfs_dev},
		{"o:", &opt_timeout, &str_timeout},
		{NULL, NULL, NULL}
	};
	
	time_t t;
	int timeout_happened = FALSE;
	

	/* Parse options. */

	if ((msg=parse_opts(argc, argv, options, &help)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	if (opt_qwheel_dev)
		qwheel_dev = str_qwheel_dev;
	if (opt_sysfs_dev)
		sysfs_dev = str_sysfs_dev;
	if (opt_timeout)
		timeout = atoi(str_timeout);


	/* Perform global test setup, call setup() function. */
	setup();


	/* Print test Assertion using tst_resm() function with argument TINFO. */
	tst_resm(TINFO, "Starting QWheel test.");
	tst_resm(TINFO, "---------------------");
	tst_resm(TINFO, "Using a timeout of %d seconds.", timeout);


	sprintf(leds, "%d", 0xff, 0);
	write(fd_sysfs_gpo_leds, leds, strlen(leds));
	
	/* Test Case Body. */
	tst_resm(TINFO, "Please press the middle key of the QWheel...");
	ret = wait_for_key_pressed(timeout);
	while (ret != MAX_WHEEL_POS+1)
	{
		if (ret < 0)
		{
			tst_resm(TFAIL, "Timeout occured.\n");
			goto clean_up;
		}
		ret = wait_for_key_pressed(timeout);
	}
	tst_resm(TINFO, "Middle key was pressed.");

	tst_resm(TINFO, "Now touch the wheel at the upper side (%d..%d)...", MAX_WHEEL_POS - 10, 10);
	sprintf(leds, "%d", (1<<0), 0);
	write(fd_sysfs_gpo_leds, leds, strlen(leds));
	ret = wait_for_key_pressed(timeout);
	while (ret < MAX_WHEEL_POS - 10 && ret > 10)
	{
		if (ret < 0)
		{
			tst_resm(TFAIL, "Timeout occured.\n");
			goto clean_up;
		}
		ret = wait_for_key_pressed(timeout);
	}
	tst_resm(TINFO, "Touch was recognized (%d).", ret);

	tst_resm(TINFO, "Now touch the wheel at the right side (%d..%d)...", MAX_WHEEL_POS/4-10, MAX_WHEEL_POS/4+10);
	sprintf(leds, "%d", (1<<2), 0);
	write(fd_sysfs_gpo_leds, leds, strlen(leds));
	ret = wait_for_key_pressed(timeout);
	while (ret < MAX_WHEEL_POS/4-10 || ret > MAX_WHEEL_POS/4+10)
	{
		if (ret < 0)
		{
			tst_resm(TFAIL, "Timeout occured.\n");
			goto clean_up;
		}
		ret = wait_for_key_pressed(timeout);
	}
	tst_resm(TINFO, "Touch was recognized (%d).", ret);

	tst_resm(TINFO, "Now touch the wheel at the bottom side (%d..%d)...", MAX_WHEEL_POS/2-10, MAX_WHEEL_POS/2+10);
	sprintf(leds, "%d", (1<<4), 0);
	write(fd_sysfs_gpo_leds, leds, strlen(leds));
	ret = wait_for_key_pressed(timeout);
	while (ret < MAX_WHEEL_POS/2-10 || ret > MAX_WHEEL_POS/2+10)
	{
		if (ret < 0)
		{
			tst_resm(TFAIL, "Timeout occured.\n");
			goto clean_up;
		}
		ret = wait_for_key_pressed(timeout);
	}
	tst_resm(TINFO, "Touch was recognized (%d).", ret);

	tst_resm(TINFO, "Now touch the wheel at the left side (%d..%d)...", MAX_WHEEL_POS*3/4-10, MAX_WHEEL_POS*3/4+10);
	sprintf(leds, "%d", (1<<6), 0);
	write(fd_sysfs_gpo_leds, leds, strlen(leds));
	ret = wait_for_key_pressed(timeout);
	while (ret < MAX_WHEEL_POS*3/4-10 || ret > MAX_WHEEL_POS*3/4+10)
	{
		if (ret < 0)
		{
			tst_resm(TFAIL, "Timeout occured.\n");
			goto clean_up;
		}
		ret = wait_for_key_pressed(timeout);
	}
	tst_resm(TINFO, "Touch was recognized (%d).", ret);
	sprintf(leds, "%d", 0, 0);
	write(fd_sysfs_gpo_leds, leds, strlen(leds));

	/* Print results and exit test-case */

	tst_resm(TPASS, "Test passed.\n");

clean_up:
	cleanup();
	tst_exit();
}