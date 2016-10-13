/******************************************************************************/
/* Copyright (C) 2010 Garz&Fricke GmbH						 			      */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        gpio.c                                                        */
/*                                                                            */
/* Description: This is a test for gpio pins.                                 */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   gpio                                                          */
/*                                                                            */
/* Test Assertion                                                             */
/* & Strategy:  Depending on the direction of the gpio pin the given value    */
/*              will be set (out) or compared to the currently read value.    */
/*              When the values are not equal or another error appears the    */
/*              test failed. Otherwise it passed.                             */
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

#define SYSFS_PATHS_LENGTH	75
#define DIRECTION_OUT		!strcmp(conf_direction, "out")
#define DIRECTION_IN		!strcmp(conf_direction, "in")

/* Extern Global Variables */
extern int  Tst_count;             	/* counter for tst_xxx routines.         */
extern char *TESTDIR;              	/* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID     = "gpio           ";		/* test program identifier.              */
int  TST_TOTAL = 1;                 	/* total number of tests in this file.   */

int  fd;								/* file descriptor of the currently opened attribute */

const char *sysfs_base_format = "/sys/class/gpio/gpio%d/";	/* format string for the gpio directory in sysfs */
char *sysfs_directory;					/* path to the sysfs directory of the given gpio */
char *sysfs_direction;					/* path to the direction attribute in sysfs of the given gpio */
char *sysfs_value;						/* path to the value attribute in sysfs of the given gpio */

int  conf_number     = 2;				/* number of the gpio pin */
char *conf_direction = "out";			/* direction of the gpio pin */
int  conf_value      = 1;				/* value to set when output pin or value to compare with when input pin */


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
	/* free the memory */
	if (sysfs_directory)	free(sysfs_directory);
	if (sysfs_direction)	free(sysfs_direction);
	if (sysfs_value)		free(sysfs_value);
	
	/* close the attribute file */
	if (close(fd) == -1) {
		tst_resm(TWARN,
			"Closing the sysfs attribute file failed. Error: %d, %s",
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
	
	/* create the paths so the sysfs attributes */
	sysfs_directory = (char *)malloc(SYSFS_PATHS_LENGTH * sizeof(char));
	sysfs_direction = (char *)malloc(SYSFS_PATHS_LENGTH * sizeof(char));
	sysfs_value     = (char *)malloc(SYSFS_PATHS_LENGTH * sizeof(char));
	
	if (!sysfs_directory || !sysfs_direction || !sysfs_value)
		tst_brkm(TBROK, cleanup,
			"Unable to allocate memory.");
			
	sprintf(sysfs_directory, sysfs_base_format, conf_number);
	strcpy(sysfs_direction, sysfs_directory);	/* append "direction" to the sysfs_directory and save in sysfs_direction */
	strcat(sysfs_direction, "direction");
	strcpy(sysfs_value, sysfs_directory);		/* append "value" to the sysfs_directory and save in sysfs_value */
	strcat(sysfs_value, "value");

	/* check whether the given gpio exists in sysfs */
	dir = opendir(sysfs_directory);
	if (!dir)
		tst_brkm(TBROK, cleanup,
			"%s does not exist. Error: %d, %s",
			sysfs_directory, errno, strerror(errno));
	closedir(dir);
	
	/* open attribute file */
	if (DIRECTION_OUT)
		fd = open(sysfs_value, O_WRONLY);
	if (DIRECTION_IN)
		fd = open(sysfs_value, O_RDONLY);
	
	if (fd == -1)
		tst_brkm(TBROK, cleanup,
			"Unable to open %s. Error: %d, %s",
			sysfs_value, errno, strerror(errno));
}


/******************************************************************************/
/*                                                                            */
/* Function:    set_value                                                     */
/*                                                                            */
/* Description: This function sets the value of an output gpio.               */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits by calling cleanup().                      */
/*              On success - Nothing.                                         */
/*                                                                            */
/******************************************************************************/
void set_value()
{
	char buf;
	
	switch(conf_value) {
		case 0: buf = '0'; break;			
		case 1: buf = '1'; break;
	}

	if (write(fd, &buf, 1) != 1) {
		tst_brkm(TBROK, cleanup,
			"Failure on writing %d to %s. Error: %d, %s",
			conf_value, sysfs_value, errno, strerror(errno));
	}
}


/******************************************************************************/
/*                                                                            */
/* Function:    get_value                                                     */
/*                                                                            */
/* Description: This function gets the value from an input gpio.              */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits by calling cleanup().                      */
/*              On success - The value read.                                  */
/*                                                                            */
/******************************************************************************/
int get_value()
{
	char buf;
	
	if (read(fd, &buf, 1) != 1) {
		tst_brkm(TBROK, cleanup,
			"Failure on reading from %s. Error: %d, %s",
			sysfs_value, errno, strerror(errno));
	}
	
	switch(buf) {
		case '0': return 0;
		case '1': return 1;
	}
}


/******************************************************************************/
/*                                                                            */
/* Function:    test                                                          */
/*                                                                            */
/* Description: This function sets an output gpio or reads an input gpio.     */
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
	if (DIRECTION_OUT)
		set_value();	
	
	if (DIRECTION_IN) {
		if (get_value() != conf_value)
			return -1;
	}
	
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
	int opt_g = 0, opt_d = 0, opt_v = 0, ret;
	char *str_g, *str_d, *str_v, *msg;
	
	option_t options[] = {
		{"g:", &opt_g, &str_g},
		{"d:", &opt_d, &str_d},
		{"v:", &opt_v, &str_v},
		{NULL, NULL, NULL}
	};	

	/* parse options */
	if ((msg = parse_opts(argc, argv, options, &help)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	if (opt_g)
		if (sscanf(str_g, "%d", &conf_number) != 1)
			tst_resm(TWARN,
				"Something wrong with the gpio pin number parameter. Using standard value 2 now.");
	if (opt_d) {		
		if (!strcmp(str_d, "out") || !strcmp(str_d, "in"))
			conf_direction = str_d;
		else
			tst_resm(TWARN,
				"Something wrong with the gpio direction parameter. Using standard value out now.");
	}
	if (opt_v) {		
		if (sscanf(str_v, "%d", &conf_value) != 1
		||	(conf_value != 0 && conf_value != 1)) {
			conf_value = 1;
			tst_resm(TWARN,
				"The value may only be 0 or 1. Using standard value 1 now.");
		}
	}
	
	/* do initialization */
	setup();

	/* start the test */
	tst_resm(TINFO,
		"Starting gpio test.",
		conf_number);
	
	/* perform the test */
	ret = test();
	
	/* evaluation */
	if (ret == -1) {
		tst_resm(TFAIL, 
			"Gotten value does not equal to given value %d. Test failed.",
			conf_value);
	}
	else if (ret == 0) {
		if (DIRECTION_OUT)			
			tst_resm(TPASS, 
				"Value %d successfully set. Test passed.",
				conf_value);		
		if (DIRECTION_IN)
			tst_resm(TPASS, 
			"Gotten value equals to given value %d. Test passed.",
			conf_value);
	}
	
	cleanup();
	tst_exit();
}