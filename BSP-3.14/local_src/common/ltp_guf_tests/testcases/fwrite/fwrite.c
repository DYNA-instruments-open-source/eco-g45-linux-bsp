/******************************************************************************/
/* Copyright (C) 2010 Garz&Fricke GmbH						 			      */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        fwrite.c                                                      */
/*                                                                            */
/* Description: This is a read/write test for filesystems.                    */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   fwrite                                                        */
/*                                                                            */
/* Test Assertion                                                             */
/* & Strategy:  fwrite is conceived to test flash memory read/write oper-     */
/*              rations. However, it can be used for every type of storage    */
/*              memory. The test creates one or mor files (depending on the   */
/*              parameter -i) with the given size and writes the alphabet     */
/*              from A to Z repeatedly into them. Afterwards, it reads back   */
/*              the data and checks whether it is identical to the data       */
/*              written. If so, the result is passed, else failed.            */
/*              If the maximum file size is reached (i.e. write() cannot      */
/*              write more bytes) a warning is displayed, but the test is     */
/*              passed, though, if the read data equals the written data.     */
/*                                                                            */
/* Author:      Tim Jaacks <tim.jaacks@garz-fricke.com                        */
/*                                                                            */
/******************************************************************************/

/* Standard Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

/* Local Defines */
#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif

#define BUFSIZE 1040000 // Buffer size approx. 1 MB (must be multiple of 26)

/* Extern Global Variables */
extern int  Tst_count;               /* counter for tst_xxx routines.         */
extern char *TESTDIR;                /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID     = "fwrite         ";  /* test program identifier.              */
int  TST_TOTAL = 1;                  /* total number of tests in this file.   */

int   fd;                            /* Handle to file                        */
unsigned long bytes = 1024;          /* Number of bytes to write              */
char* file_name = "testfile";        /* String containing the file name       */
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
	/* Free all allocated memory. */

	/* Close all open file descriptors. */

	/* Remove all temporary directories used by this test. */

	// Insert real code here 

	/* kill child processes if any. */

	// Insert code here
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
	int i, fd;
	char *pos_of_slash;
	
	// Fill write buffer with alphabet
	write_buffer[0] = 'A';
	for (i=1; i<BUFSIZE; i++)
	{
		write_buffer[i] = write_buffer[i-1] + 1;
		if (write_buffer[i] > 'Z')
			write_buffer[i] = 'A';
	}
	
	// Check whether directory exists. If not, create it.
	pos_of_slash = strrchr(file_name, '/');
	if (pos_of_slash != NULL)
	{
		*pos_of_slash = 0;
		if (strlen(file_name) > 0)
		{
			fd = open(file_name, O_RDONLY);
			if (fd < 0)
				mkdir(file_name,0777);
			else
				close(fd);
		}
		*pos_of_slash = '/';
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
	printf("  -n f    Write to file f (default \"testfile\")\n");
	printf("  -d      Delete the created file(s)\n");
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
	int opt_bytes = 0, opt_filename = 0, opt_delete = 0, opt_suppress_info = 0;
	char *str_bytes, *str_filename, *msg;
	int lc;		/* loop counter */

	unsigned long bytes_written;
	unsigned long bytes_read;
	int i;
	unsigned long msec;
	struct timeval tv_start, tv_end;
	char this_file_name[PATH_MAX];
	
	option_t options[] = {
		{"n:", &opt_filename, &str_filename},
		{"b:", &opt_bytes, &str_bytes},
		{"d", &opt_delete, NULL},
		{"q", &opt_suppress_info, NULL},
		{NULL, NULL, NULL}
	};
	
	time_t t;

	/* Parse options. */
	if ((msg=parse_opts(argc, argv, options, &help)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	if (opt_filename)
		file_name = str_filename;
	if (opt_bytes)
		bytes = strtoul(str_bytes, NULL, 0);


	/* Perform global test setup, call setup() function. */
	setup();

	/* Test Case Body. */
	tst_resm(TINFO, "Creating %d files with %lu bytes each...", STD_LOOP_COUNT, bytes);

	for (lc=0; TEST_LOOPING(lc) && !failed; lc++) {
		sprintf(this_file_name, "%s_%d", file_name, Tst_count);
		remove(this_file_name);
		
		bytes_written = 0;
		bytes_read = 0;
		
		// Write cached data to disk
		sync();
		system("echo 3 > /proc/sys/vm/drop_caches");

		fd = open (this_file_name, O_WRONLY | O_CREAT | O_SYNC);
		
		if (fd < 0)
		{
			tst_resm(TBROK, "open(%s) Failed, errno=%d : %s",
				this_file_name, errno, strerror(errno));
			tst_exit();
		}
		
		// Write file
		gettimeofday(&tv_start, NULL);
		if (!opt_suppress_info)
			tst_resm(TINFO, "Writing %lu bytes to file %s...", bytes, this_file_name);
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
		gettimeofday(&tv_end, NULL);

		msec = (tv_end.tv_sec - tv_start.tv_sec)*1000 + (tv_end.tv_usec - tv_start.tv_usec)/1000;

		if (!opt_suppress_info)
		{
			tst_resm(TINFO, "Data written in %lu milliseconds (%lu kB/s)", msec, bytes/(msec));
			tst_resm(TINFO, "Reading back data for comparison...");
		}
		
		if (close(fd) == -1)
		{
			tst_resm(TWARN, "close(%s) Failed, errno=%d : %s",
				this_file_name, errno, strerror(errno));
		}

		// Write cached data to disk
		sync();
		system("echo 3 > /proc/sys/vm/drop_caches");
	
		// Fill write buffer with alphabet
		write_buffer[0] = 'A';
		for (i=1; i<BUFSIZE; i++)
		{
			write_buffer[i] = write_buffer[i-1] + 1;
			if (write_buffer[i] > 'Z')
				write_buffer[i] = 'A';
		}
		
		// Read and compare file
		fd = open (this_file_name, O_RDONLY | O_SYNC);
		if (fd < 0)
		{
			tst_resm(TBROK, "open(%s) Failed, errno=%d : %s",
				this_file_name, errno, strerror(errno));
			tst_exit();
		}
		
		while((bytes_read = read(fd, read_buffer, BUFSIZE)) > 0)
		{
			for (i=0; i<bytes_read; i++)
				if (write_buffer[i] != read_buffer[i])
				{
					//printf("%c != %c (i = %d)\n", write_buffer[i], read_buffer[i], i);
					failed = TRUE;
					break;
				}
		}
		
		if (close(fd) == -1)
		{
			tst_resm(TWARN, "close(%s) Failed, errno=%d : %s",
				this_file_name, errno, strerror(errno));
		}

		/* Print results and exit test-case */
		if (!opt_suppress_info)
		{
			if (failed)
				tst_resm(TFAIL, "Read/write error. Test failed.");
			else
				tst_resm(TPASS, "Data was correctly written and read back. Test passed.");
		}

		if (opt_delete)
		{
			if (!opt_suppress_info)
				tst_resm(TINFO, "removing file");
			remove(this_file_name);
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
}
