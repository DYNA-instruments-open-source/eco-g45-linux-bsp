/******************************************************************************/
/* Copyright (C) 2011 Garz&Fricke GmbH						 			      */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        bwrite.c                                                      */
/*                                                                            */
/* Description: This is a read/write test for block devices.                  */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   fwrite                                                        */
/*                                                                            */
/* Test Assertion & Strategy:                                                 */
/*              bwrite is conceived to test block device read/write oper-     */
/*              rations. The test writes a number ...						  */
/*                                                                            */
/* Author:      Carsten Behling <carsten.behling@garz-fricke.com>             */
/*                                                                            */
/******************************************************************************/

/* Standard Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

/* Local Defines */
#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif

#define BUFSIZE 1040000 /* Buffer size approx. 1 MB (must be multiple of 26) */

/* Extern Global Variables */
extern int  Tst_count;               /* counter for tst_xxx routines.         */
extern char *TESTDIR;                /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID     = "bwrite         ";  /* test program identifier.              */
int  TST_TOTAL = 1;                  /* total number of tests in this file.   */
int opt_bytes = 0, opt_filename = 0, opt_suppress_info = 0;
char *str_bytes, *str_filename;


int   fd;                            /* Handle to file                        */
unsigned long bytes = 1024;          /* Number of bytes to write              */
char* device_name = "/dev/sda";      /* String containing the block device    */
char  write_buffer[BUFSIZE];         /* Buffer to be written                  */
char  read_buffer[BUFSIZE];          /* Buffer to be read                     */
int   failed = FALSE;


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
	/* Nothing to do here */
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
    struct stat st;
	unsigned int i = 0;
	
	/* Fill write buffer with alphabet. */
	write_buffer[0] = 'A';
	for (i=1; i<BUFSIZE; i++)
	{
		write_buffer[i] = write_buffer[i-1] + 1;
		if (write_buffer[i] > 'Z')
			write_buffer[i] = 'A';
	}
	
	/* Check if the device node exists. */
	if (-1 == stat(device_name, &st)) {
		tst_brkm(TBROK, cleanup,
			"Cannot identify block device '%s': %d, %s",
			device_name, errno, strerror(errno));
	}

	/* Check if the device node belongs to a block device. */
	if (!S_ISBLK(st.st_mode)) {
		tst_brkm(TBROK, cleanup,
			"'%s'is not a block device.",
			device_name);
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
	printf("  -b x    Write x bytes (default 1024)\n");
	printf("  -n f    Write to device f (default \"/dev/sda\")\n");
	printf("  -q      Suppress info messages\n");
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
/*               -i - Number of iteration                                     */
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
	int lc;		/* loop counter */
	unsigned long bytes_written;
	unsigned long bytes_read;
	unsigned long bytes_to_read;
	int i;
	unsigned long msec;
	struct timeval tv_start, tv_end;
	char this_device_name[PATH_MAX];
	char *msg;
	
	option_t options[] = {
		{"n:", &opt_filename, &str_filename},
		{"b:", &opt_bytes, &str_bytes},
		{"q", &opt_suppress_info, NULL},
		{NULL, NULL, NULL}
	};

	/* Parse options. */
	if ((msg=parse_opts(argc, argv, options, &help)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	if (opt_filename)
		device_name = str_filename;
	if (opt_bytes)
		bytes = strtoul(str_bytes, NULL, 0);

		
	/* Perform global test setup, call setup() function. */
	setup();

	/* Test Case Body. */
	tst_resm(TINFO, "Writing %d times %lu bytes to \"%s\"...", STD_LOOP_COUNT, bytes, device_name);

	for (lc=0; TEST_LOOPING(lc) && !failed; lc++) {		
		bytes_written = 0;
		bytes_read = 0;

		/* Write cached data to the block device. */
		sync();

		/* Flush caches to be sure to read directly from the block device. */
		if(-1 == system("echo 3 > /proc/sys/vm/drop_caches"))
		{
			tst_resm(TBROK, "flush cache failed");
			tst_exit();		
		}
		
		/* Open the block device for writing. */
		fd = open (device_name, O_WRONLY | O_SYNC);
		
		if (-1 == fd)
		{
			tst_resm(TBROK, "open(%s) failed, errno=%d : %s",
				this_device_name, errno, strerror(errno));
			tst_exit();
		}
		
		/* Write the test data. */
		if(-1 == gettimeofday(&tv_start, NULL)) {
			tst_resm(TBROK, "gettimeofday failed, errno=%d : %s",
				errno, strerror(errno));		
			tst_exit();
		}
		
		if (!opt_suppress_info)
			tst_resm(TINFO, "Writing %lu bytes to file %s...", bytes, device_name);


		while(bytes_written < bytes)
		{
			unsigned long bytes_to_write = bytes - bytes_written;
			int ret;
			if (bytes_to_write > BUFSIZE)
				bytes_to_write = BUFSIZE;
			ret = write(fd, write_buffer, bytes_to_write);
			if (ret > 0)
				bytes_written += ret;
			if (ret != bytes_to_write)
			{
				tst_resm(TINFO, "Could not write any more bytes! (Bytes written: %lu)", bytes_written);
				break;
			}
		}
		
		if(-1 == gettimeofday(&tv_end, NULL)) {
			tst_resm(TBROK, "gettimeofday failed, errno=%d : %s",
				errno, strerror(errno));		
				tst_exit();
		}

		msec = (tv_end.tv_sec - tv_start.tv_sec)*1000 + (tv_end.tv_usec - tv_start.tv_usec)/1000;
		
		if (!opt_suppress_info)
		{
			tst_resm(TINFO, "Data written in %lu milliseconds (%lu kB/s)", msec, bytes/(msec));
			tst_resm(TINFO, "Reading back data for comparison...");
		}
		
		if (-1 == close(fd))
		{
			tst_resm(TBROK, "close(%s) failed, errno=%d : %s",
				this_device_name, errno, strerror(errno));
			tst_exit();
		}

		/* Write cached data to the block device. */
		sync();

		/* Flush caches to be sure to read directly from the block device. */
		if(-1 == system("echo 3 > /proc/sys/vm/drop_caches"))
		{
			tst_resm(TBROK, "flush cache failed");
			tst_exit();		
		}

		/* Read and compare file. */
		fd = open (device_name, O_RDONLY | O_SYNC);
		if (fd < 0)
		{
			tst_resm(TBROK, "open(%s) failed, errno=%d : %s",
				this_device_name, errno, strerror(errno));
			tst_exit();
		}

		bytes_to_read = bytes;
		while(0 != bytes_to_read)
		{
			bytes_read = read(fd, read_buffer, bytes);
			bytes_to_read -= bytes_read;
			for (i = 0; i < bytes_read; i++)
				if (write_buffer[i] != read_buffer[i])
				{
					printf("%c != %c (i = %d)\n", write_buffer[i], read_buffer[i], i);
					failed = TRUE;
					break;
				}
		}

		if (-1 == close(fd))
		{
			tst_resm(TBROK, "close(%s) failed, errno=%d : %s",
				this_device_name, errno, strerror(errno));
			tst_exit();
		}

		/* Print results and exit test-case */
		if (!opt_suppress_info)
		{
			if (failed)
				tst_resm(TFAIL, "Read/write error. Test failed.");
			else
				tst_resm(TPASS, "Data was correctly written and read back. Test passed.");
		}
	}

	if (opt_suppress_info)
	{
		if (failed)
			tst_resm(TFAIL, "Read/write error. Test failed.");
		else
			tst_resm(TPASS, "All files were correctly written and read back. Test passed.");
	}

	cleanup();

	tst_exit();
	return 0;
}
