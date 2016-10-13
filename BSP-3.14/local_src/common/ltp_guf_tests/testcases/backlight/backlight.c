/******************************************************************************/
/* Copyright (C) 2010 Garz&Fricke GmbH						 			      */
/*		No use or disclosure of this information in any form 				  */
/*		without	the written permission of the author					      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        backlight.c                                          	      */
/*                                                                            */
/* Description: This is a backlight test.                							*/
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:	backlight                                               	  */
/*                                                                            */
/* Test Assertion   In this testcase, the backlight brightness and power      */
/* & Strategy:  	get tested. First the brightness is set to min -> 0  	  */
/*					and then incremented with the value of 15. So there are	  */
/*					18 brightness steps.									  */
/*					Next the backlight power will be tested. 				  */
/*					If power is on	: first off -> on						  */
/*					If Power is off	: first on -> off						  */
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
#include <sys/wait.h>
#include <linux/fs.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

#define BUF		5


/* Local Defines */
#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif

/* Extern Global Variables */
extern int  Tst_count;              	/* counter for tst_xxx routines.         			*/
extern char *TESTDIR;               	/* temporary dir created by tst_tmpdir() 			*/

/* Global Variables */
char *TCID     = "backlight      ";  	/* test program identifier.              			*/
int  TST_TOTAL = 1;                  	/* total number of tests in this file.   			*/

int fd;
int repetitions = 1;					/* default repetition for loop_count				*/

char* f_brightness = "/sys/class/backlight/pwm-backlight/brightness";
char* f_blpower = "/sys/class/backlight/pwm-backlight/bl_power";



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
	printf("  -r x    Repeat send and receive x times (default: 1)\r\n");
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
	char wbuffer[BUF];	/* buffer for write operations */
	char rbuffer[BUF];	/* buffer for read operations  */
	
	int brightness = 0;
	int check_count = 0;
	int loop_count = 0;				/* loop counter for testcase*/
	int lc;
	
	int opt_repetitions = 0;
	char *str_repetitions, *msg;
	

	/* for test specific parse_opts options, needed for using the stadard options calles with -h */
	option_t options[] = {
		{"r:", &opt_repetitions, &str_repetitions},
		{NULL, NULL, NULL}
	};
		
	/* parse options */
		
	if ((msg=parse_opts(argc, argv, options, &help)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	
	if (opt_repetitions)
		repetitions = atoi(str_repetitions);
	
	
	for (loop_count=0; loop_count < repetitions; loop_count++) 
	//for (lc=0; TEST_LOOPING(lc); lc++)
	{

	/* reset Tst_count in case we are looping. */
	// printf("der counter für test. %d\n",loop_count);
	Tst_count = 0;
	
	/****************************************************/
	tst_resm(TINFO, "Starting BACKLIGHT BRIGHTNESS test.");
	
	fd = open(f_brightness, O_RDWR);
	if (!fd) 
	    tst_brkm(TBROK, cleanup, " Cannot open the file %s . Error:%d, %s\n",f_brightness,
			errno, strerror(errno));
	
	/* increment the brightness value from 0 --> 255 */
	for(brightness = 0,check_count = 0; brightness <= 255; brightness=brightness+15)
	{	
		/* sleep needed, otherwise its to fast */
		usleep(500000);
		sprintf(wbuffer,"%d\n",brightness);
			if((write(fd,wbuffer,4)) == -1)
				tst_brkm(TBROK, cleanup, " Cannot write into file %s . Error:%d, %s\n",f_brightness,
							errno, strerror(errno));
						
		tst_resm(TINFO, "Setting brightness to %d", brightness);
		check_count++;
		}	
	
		if(check_count != 18)
			tst_resm(TFAIL, "Backlight brightness test, does not end correctly. Test FAILED.");
		if (close(fd) == -1)
		{
			tst_resm(TWARN, "close(%s) Failed, errno=%d : %s",
			f_brightness, errno, strerror(errno));
		}
	
		tst_resm(TPASS, "Backlight brightness test PASSED.");
	
		/****************************************************/
		tst_resm(TINFO, "Starting BACKLIGHT POWER test.");
	
		fd = open(f_blpower, O_RDWR);
		if (!fd) 
			tst_brkm(TBROK, cleanup, " Cannot open the file %s . Error:%d, %s\n",f_blpower,
						errno, strerror(errno));
	
	
		rbuffer[read(fd,rbuffer,BUF)] = 0;
		/* looks if power is on or off. default is ON */
		if(strstr(rbuffer,"0"))
		{
			sprintf(wbuffer,"1\n");
			if((write(fd,wbuffer,4)) == -1)
				tst_brkm(TBROK, cleanup, " Cannot write into file %s . Error:%d, %s\n",f_blpower,
							errno, strerror(errno));
			/* sleep needed, otherwise its to fast */				
			sleep(1);
			
			sprintf(wbuffer,"0\n");
			if((write(fd,wbuffer,4)) == -1)
				tst_brkm(TBROK, cleanup, " Cannot write into file %s . Error:%d, %s\n",f_blpower,
							errno, strerror(errno));
							
		}else{
			sprintf(wbuffer,"0\n");
			if((write(fd,wbuffer,4)) == -1)
				tst_brkm(TBROK, cleanup, " Cannot write into file %s . Error:%d, %s\n",f_blpower,
							errno, strerror(errno));
			/* sleep needed, otherwise its to fast */				
			sleep(1);
			
			sprintf(wbuffer,"1\n");
			if((write(fd,wbuffer,4)) == -1)
				tst_brkm(TBROK, cleanup, " Cannot write into file %s . Error:%d, %s\n",f_blpower,
							errno, strerror(errno));
		}
				
	
		if (close(fd) == -1)
		{
			tst_resm(TWARN, "close(%s) Failed, errno=%d : %s",
			f_blpower, errno, strerror(errno));
		}
	
		tst_resm(TPASS, "Backlight power test PASSED.");	
	}
	cleanup();
	tst_exit();
}