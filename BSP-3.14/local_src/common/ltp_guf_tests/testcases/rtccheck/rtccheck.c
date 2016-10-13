/******************************************************************************/
/* Copyright (C) 2010 Garz&Fricke GmbH						 			      */
/*		No use or disclosure of this information in any form 				  */
/*		without	the written permission of the author					      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        rtccheck.c                                               */
/*                                                                            */
/* Description: This is a test for the RTC (Real-Time-Control)                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   rtccheck                                                 */
/*                                                                            */
/* Test Assertion                                                             */
/* & Strategy:  */
/*                                                                            */
/* Author:      Sakire Aytac <sakire.aytac@garz-fricke.com                    */
/*                                                                            */
/******************************************************************************/

/* Standard Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <asm/ioctls.h>
#include <linux/serial.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

/* Local Defines */
#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif

/* Extern Global Variables */
extern int  Tst_count;              		/* counter for tst_xxx routines.         			*/
extern char *TESTDIR;               		/* temporary dir created by tst_tmpdir() 			*/

/* Global Variables */
char *TCID     = "rtccheck       ";	 		/* test program identifier.              			*/
int  TST_TOTAL = 1;                  		/* total number of tests in this file.   			*/

char* current_time = "2010.01.01-01:01";	/* string for setting current Time 		  			*/
char dateset[50]="date -s \"";				/* string for set time with data "<current_time>"   */
int state=1;								/* inherits the current state 						*/
int delay_time = 60;						/* variable for calculated difftime					*/


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

void setup()
{

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

	printf("  -v s  enter the date in format YYYY.MM.DD-HH:MM.(default: 01.01.2010-01:01)\n");
	printf("  -d x  enter the delay x for clock test.(default: 60sec)\n");
	
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
int main(	int   argc,    /* number of command line parameters                      */
			char **argv)   /* pointer to the array of the command line parameters.   */
{
	int sec_sw,sec_hw,diff_swhw;
	time_t now;
	struct tm time_stamp;

	/* To add command line options you need to declare a structure to pass to
	* parse_opts().  options is the structure used in this example.  The format is
	* the string that should be added to optstring in getopt(3), an integer that
	* will be used as a flag if the option is given, and a pointer to a string that
	* should receive the optarg parameter from getopt(3).  Here we add a -N
	* option.  Long options are not supported at this time. 
	*/
 
	int opt_current_time = 0, opt_delay_time = 0;
	char *str_current_time, *str_delay_time, *msg;


	/* for test specific parse_opts options */
	option_t options[] = {
		{"v:", &opt_current_time, &str_current_time},
		{"d:", &opt_delay_time, &str_delay_time},
		{NULL, NULL, NULL}
	};
	
		
	if ((msg=parse_opts(argc, argv, options, &help)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	if (opt_current_time)
		current_time = str_current_time;
	if (opt_delay_time)
		delay_time = atoi(str_delay_time);
	
	/* Print test Assertion using tst_resm() function with argument TINFO. */
	tst_resm(TINFO, "Starting RTC-CHECK test.");
	
	/*   set time to 01.01.1970 00:00:00 SystemClock AND HardwareClock     */
					
	tst_resm(TINFO, "Setting time to 01.01.1970 for RTC-Check.");
	system("date -s \"1970.01.01-00:00:00\" >/dev/null");
	system("hwclock --systohc");
															
	/* setting the current_time */					
	tst_resm(TINFO, "Setting Systemclock to: %s", current_time);
	
	strcat(dateset,current_time);
	strcat(dateset,"\" >/dev/null");

	system(dateset);
										
	/* setting hwclock to current_time and Wait */
	tst_resm(TINFO, "Setting HW clock and wait %d sec.",delay_time);
	system("hwclock --systohc");
	sleep(delay_time);
								
	/* check diffrence between HW and SW clock */
	tst_resm(TINFO, "Check the time difference between HW and SW clock.");
	
	time(&now);
	time_stamp = *localtime(&now);
	
	sec_sw = time_stamp.tm_sec;
	/* test if diffrence is greater then 3 seconds */
	/* sleep(4);*/
	system("hwclock --hctosys");
	time(&now);
	time_stamp = *localtime(&now);
	sec_hw = time_stamp.tm_sec;
	diff_swhw=sec_hw-sec_sw;
	
	/* for showing the diffrence seconds */
	/*printf("Diff: %d \n", (diff_swhw)); */

	
	if(diff_swhw >3 ||diff_swhw <(-3))
		tst_resm(TFAIL, "Time diffrence HW & SW-Clock is to much(%d sec). Test FAILED.",diff_swhw);
	else
		tst_resm(TPASS, "Time diffrence HW&SW-Clock is fine. Test PASSED.");
						
						//printf("Diff: %d \n", (diff_swhw));

	/* for negative diff time in seconds */
	if(diff_swhw <0)
		diff_swhw=diff_swhw*(-1);
	
	cleanup();
	tst_exit();
}