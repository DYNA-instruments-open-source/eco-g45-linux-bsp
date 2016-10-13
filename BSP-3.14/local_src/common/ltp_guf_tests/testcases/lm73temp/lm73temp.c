/******************************************************************************/
/* Copyright (C) 2012 Garz&Fricke GmbH						 			      */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        lm73temp.c                                                    */
/*                                                                            */
/* Description: This is a test for the LM73x temperature sensor.              */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   lm73temp                                                      */
/*                                                                            */
/* Test Assertion                                                             */
/* & Strategy:  The test just reads the current temperature value from the    */
/*              given sysfs file. This file is created by the LM73 kernel     */
/*              driver.                                                       */
/*                                                                            */
/* Author:      Phillip Durdaut <phillip.durdaut@garz-fricke.com              */
/*                                                                            */
/******************************************************************************/

/* Standard Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

/* Local Defines */
#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif

#define TEMP_MAX_LENGTH 10
#define SYSFS_DIRECTORY	"/sys/bus/i2c/drivers/lm73/0-0049"				/* path to the sysfs directory of the sensor */
#define SYSFS_FILE		"/sys/bus/i2c/drivers/lm73/0-0049/temp1_input"	/* path to the the temperature attribute file */

/* Extern Global Variables */
extern int  Tst_count;             	/* counter for tst_xxx routines.         */
extern char *TESTDIR;              	/* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID     = "lm73temp       ";		/* test program identifier.              */
int  TST_TOTAL = 1;                 	/* total number of tests in this file.   */

int  fd;								/* file descriptor of the currently opened attribute */

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
extern void cleanup()
{
	/* close the attribute file */
	if (close(fd) == -1) {
		tst_resm(TWARN,
			"Closing the sysfs file failed. Error: %d, %s",
			errno, strerror(errno));
	}

	tst_exit();
}


/* Local Functions */
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
/*              On success - Nothing.                                         */
/*                                                                            */
/******************************************************************************/
void setup()
{
	DIR *dir;

	/* check whether the directory exists in sysfs */
	dir = opendir(SYSFS_DIRECTORY);
	if (!dir)
		tst_brkm(TBROK, cleanup,
			"%s does not exist. Error: %d, %s",
			SYSFS_DIRECTORY, errno, strerror(errno));
	closedir(dir);

	/* open attribute file */
	fd = open(SYSFS_FILE, O_RDONLY);
	if (fd == -1)
		tst_brkm(TBROK, cleanup,
			"Unable to open %s. Error: %d, %s",
			SYSFS_FILE, errno, strerror(errno));
}


/******************************************************************************/
/*                                                                            */
/* Function:    test                                                          */
/*                                                                            */
/* Description: This function reads the temperature from the sensors sysfs    */
/*              file. Before the temperature is shown on the terminal, the    */
/*              string gets finalized with a zero.                            */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - returns -1.                                      */
/*              On success - returns 0.                                       */
/*                                                                            */
/******************************************************************************/
int test()
{
	char temp[TEMP_MAX_LENGTH];
	int i = 0;

	/* clean the memory */
	for (i = 0; i < TEMP_MAX_LENGTH; i++)
		temp[i] = 0;

	if (read(fd, &temp, TEMP_MAX_LENGTH) == -1) {
		tst_brkm(TBROK, cleanup,
			"Failure on reading from %s. Error: %d, %s",
			SYSFS_FILE, errno, strerror(errno));
		return -1;
	}

	/* allowed characters: +, -, ., 0..9 */
	for (i = 0; i < TEMP_MAX_LENGTH; i++) {
		if (!(temp[i] == '+' || temp[i] == '-' || temp[i] == '.' || (temp[i] > 47 && temp[i] < 58)))
			temp[i] = 0;
	}

	tst_resm(TINFO, "%s degree Celsius", temp);
	return 0;
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
	printf("  -g n    Select the gpio pin to use by its number n (default: 2)\r\n");
	printf("  -d s    Select whether the direction of the pin is in or out (default: out)\r\n");
	printf("  -v x    Select the value to set (out) or the value to compare with (in) (default: 1)\r\n");
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
/* Input:       Command line parameters (see help())                          */
/*                                                                            */
/* Exit:        On failure - Exits by calling cleanup().                      */
/*              On success - exits with 0 exit value.                         */
/*                                                                            */
/******************************************************************************/
int main(int argc, char **argv)
{
	int ret, lc;
	char *msg;

	option_t options[] = {
		{NULL, NULL, NULL}
	};

	/* parse options */
	if ((msg = parse_opts(argc, argv, options, &help)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	/* start the test */
	tst_resm(TINFO,
		"Starting lm73temp test.");

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* do initialization */
		setup();

		/* perform the test */
		ret = test();

		/* evaluation */
		if (ret == -1) {
			tst_resm(TFAIL, "Test failed.");
		}
		else if (ret == 0) {
			tst_resm(TPASS, "Test passed.");
		}
	}

	cleanup();
	tst_exit();
}