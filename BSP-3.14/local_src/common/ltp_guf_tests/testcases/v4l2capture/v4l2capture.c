/******************************************************************************/
/* Copyright (C) 2011 Garz & Fricke GmbH						 			  */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        v4l2capture.c                                                 */
/*                                                                            */
/* Description: This is a test for v4l2 capture interfaces (e.g. cameras).    */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   v4l2                                                          */
/*                                                                            */
/* Test Assertion                                                             */
/* & Strategy:  TODO  								                          */
/*                                                                            */
/* Author:      Carsten Behling <carsten.behling@garz-fricke.com              */
/*                                                                            */
/******************************************************************************/

/* Standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

/* Harness specific include files. */
#include "test.h"
#include "usctest.h"

/* Local include files */
#include "v4l2capture.h"
#include "framebuffer.h"
#include "capture.h"

/* Extern global variables */
extern int  Tst_count; /* counter for tst_xxx routines.         */
extern char *TESTDIR;  /* temporary dir created by tst_tmpdir() */

/* Global variables */
char *TCID     = "v4l2capture    "; /* test program identifier. */
int  TST_TOTAL = 1; /* total number of tests in this file. */

/* Extern global functions */
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
extern void cleanup(void)
{
	if(ERROR_MUNMAP == uninit_capture_device()) {
		tst_resm(TWARN,
			"Uninitializing the capture device '%s' failed. Error: %d, %s",
			conf_device_name_cap, errno, strerror(errno));
	}
	if(ERROR_CLOSE == close_capture_device()) {
		tst_resm(TWARN,
			"Closing the capture device '%s' failed. Error: %d, %s",
			conf_device_name_cap, errno, strerror(errno));
	}
	if(ERROR_MUNMAP == uninit_framebuffer_device()) {
		tst_resm(TWARN,
			"Uninitializing the framebuffer device '%s' failed. Error: %d, %s",
			conf_device_name_fb, errno, strerror(errno));
	}
	if(ERROR_CLOSE == close_framebuffer_device()) {
		tst_resm(TWARN,
			"Closing the framebuffer device '%s' failed. Error: %d, %s",
			conf_device_name_fb, errno, strerror(errno));
	}
	tst_exit();
}

/* Local functions */
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
	error_t ret = SUCCESS;

	ret = open_capture_device();
	switch(ret) {
		case SUCCESS:
		break;
		case ERROR_STAT:
			tst_brkm(TBROK, cleanup,
				"Cannot identify capture device '%s': %d, %s\n",
				conf_device_name_cap, errno, strerror(errno));
		break;
		case ERROR_ISCHR:
			tst_brkm(TBROK, cleanup,
				"'%s'is not a device.\n",
				conf_device_name_cap);
		break;
		case ERROR_OPEN:
			tst_brkm(TBROK, cleanup,
				"Unable to open the capture device %s. Error: %d, %s",
				conf_device_name_cap, errno, strerror(errno));		
		default:
			tst_brkm(TBROK, cleanup,
				"Unable to access '%s'. Unknown error.\n",
				conf_device_name_cap);
		break;
	}

	ret = init_capture_device();
	switch(ret) {
		case SUCCESS:
		break;
		case ERROR_MMAP:
			tst_brkm(TBROK, cleanup,
				"'MMAP' for device '%s' failed: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
		break;
		case ERROR_VIDIOC_QUERYCAP:
			if (EINVAL == errno) {
				tst_brkm(TBROK, cleanup,
					"'%s'is not a Video4Linux2 device.\n",
					conf_device_name_cap);
			} else {
				tst_brkm(TBROK, cleanup,
					"IOCTL 'VIDIOC_QUERYCAP' for device '%s' failed: %d, %s.\n",
					conf_device_name_cap, errno, strerror(errno));
			}
		break;
		case ERROR_VIDIOC_CROPCAP:
			tst_resm(TWARN,
				"IOCTL 'VIDIOC_CROPCAP' for device '%s' failed: %d, %s\n",
				conf_device_name_cap, errno, strerror(errno));
		break;
		case ERROR_VIDIOC_S_CROP: 
            switch (errno) {
                case EINVAL:
				tst_resm(TWARN,
					"Cropping for capture device '%s'is not supported.\n",
					conf_device_name_cap);
                break;
                default:
				tst_resm(TWARN,
					"Cropping error for capture device '%s' occured.\n",
					conf_device_name_cap);
                break;
            }
  		break;
		case ERROR_VIDIOC_S_FMT:
			tst_brkm(TBROK, cleanup,
				"IOCTL 'VIDIOC_S_FMT' for device '%s' failed: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
		break;
		case ERROR_VIDIOC_REQBUFS:
			if (EINVAL == errno) {
				tst_brkm(TBROK, cleanup,
					"'%s'does not support I/O method 'mmap'.\n",
					conf_device_name_cap);
			} else {
				tst_brkm(TBROK, cleanup,
					"IOCTL 'VIDIOC_QUERYCAP' for device '%s' failed: %d, %s.\n",
					conf_device_name_cap, errno, strerror(errno));
			}
 		break;
		case ERROR_VIDIOC_QUERYBUF:
			tst_brkm(TBROK, cleanup,
				"IOCTL 'VIDIOC_QUERYBUF' for device '%s' failed: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
		break;
		case ERROR_VID_NOT_A_CAP_DEV:
			tst_brkm(TBROK, cleanup,
				"'%s'is not a Video4Linux2 capture device.\n",
				conf_device_name_cap);
		break;
		case ERROR_VID_NOT_IO_READ:
			tst_brkm(TBROK, cleanup,
				"'%s' does not support selected I/O method 'read'.\n",
				conf_device_name_cap);
		case ERROR_VID_NOT_IO_STREAM:
			tst_brkm(TBROK, cleanup,
				"'%s' does not support selected I/O method 'streaming' for 'mmap' or 'userptr'.\n",
				conf_device_name_cap);
		case ERROR_VID_NOT_ENOUGH_IO_BUF:
			tst_brkm(TBROK, cleanup,
				"Could not allocate sufficient I/O buffers for device '%s.\n",
				conf_device_name_cap);
		break;
		case ERROR_CALLOC:
			tst_brkm(TBROK, cleanup,
				"'CALLOC' for device '%s' failed: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
		break;
		case ERROR_MALLOC:
			tst_brkm(TBROK, cleanup,
				"'MALLOC' for device '%s' failed: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
		break;
		case ERROR_MEMALIGN:
			tst_brkm(TBROK, cleanup,
				"'MEMALIGN' for device '%s' failed: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
		break;
		default:
			tst_brkm(TBROK, cleanup,
				"Unable to access '%s'. Unknown error.\n",
				conf_device_name_cap);
		break;
	}

	ret = open_framebuffer_device();
	switch(ret) {
		case SUCCESS:
		break;
		case ERROR_STAT:
			tst_brkm(TBROK, cleanup,
				"Cannot identify framebuffer device '%s': %d, %s\n",
				conf_device_name_fb, errno, strerror(errno));
		break;
		case ERROR_ISCHR:
			tst_brkm(TBROK, cleanup,
				"'%s'is not a device.\n",
				conf_device_name_fb);
		break;
		case ERROR_OPEN:
			tst_brkm(TBROK, cleanup,
				"Unable to open the framebuffer device %s. Error: %d, %s",
				conf_device_name_fb, errno, strerror(errno));		
		default:
			tst_brkm(TBROK, cleanup,
				"Unable to access '%s'. Unknown error.\n",
				conf_device_name_fb);
		break;
	}
	
	ret = init_framebuffer_device();
	switch(ret) {
		case SUCCESS:
		break;
		case ERROR_FBIOGET_FSCREENINFO:
			tst_brkm(TBROK, cleanup,
				"IOCTL 'FBIOGET_FSCREENINFO' for device '%s' failed: %d, %s.\n",
				conf_device_name_fb, errno, strerror(errno));
		break;
		case ERROR_FBIOGET_VSCREENINFO:
			tst_brkm(TBROK, cleanup,
				"IOCTL 'FBIOGET_VSCREENINFO' for device '%s' failed: %d, %s.\n",
				conf_device_name_fb, errno, strerror(errno));
		break;
		case ERROR_MMAP:
			tst_brkm(TBROK, cleanup,
				"'MMAP' for device '%s' failed: %d, %s.\n",
				conf_device_name_fb, errno, strerror(errno));
		break;
		default:
			tst_brkm(TBROK, cleanup,
				"Unable to access '%s'. Unknown error.\n",
				conf_device_name_fb);
		break;
	}
}

/******************************************************************************/
/*                                                                            */
/* Function:    test                                                          */
/*                                                                            */
/* Description: TODO                                                          */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - returns -1.                                      */
/*              On success - returns 0.                                       */
/*                                                                            */
/******************************************************************************/
int test(void)
{
	error_t ret = SUCCESS;
	int result = -1;

	ret = start_capturing();
	switch(ret) {
		case SUCCESS:
			result = 0;
		break;
		case ERROR_VIDIOC_QBUF:
			tst_resm(TINFO,
				"IOCTL 'VIDIOC_QBUF' for device '%s' failed: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
			result = -1;
		break;
		case ERROR_VIDIOC_STREAMON:
			tst_resm(TINFO,
				"IOCTL 'VIDIOC_STREAMON' for device '%s' failed: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
			result = -1;
		break;
		default:
			tst_resm(TINFO,
				"Could not start capturing from device '%s'. Unknown error.\n",
				conf_device_name_cap);
			result = -1;
		break;
	}
	
	if(ret)
		goto err0;
		
	ret = capture_mainloop();
	switch(ret) {
		case SUCCESS:
			result = 0;
		break;
		case ERROR_SELECT:
			tst_resm(TINFO,
				"'select' system call for device '%s' failed: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
			result = -1;
		break;
		case ERROR_TIMEOUT:
			tst_resm(TINFO,
				"'select' system call for device '%s' timed out: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
			result = -1;
		break;
		case ERROR_READ:
			tst_resm(TINFO,
				"'read' system call for device '%s' failed: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
			result = -1;
		break;
		case ERROR_VIDIOC_QBUF:
			tst_resm(TINFO,
				"IOCTL 'VIDIOC_QBUF' for device '%s' failed: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
			result = -1;
		break;
		case ERROR_VIDIOC_DQBUF:
			tst_resm(TINFO,
				"IOCTL 'VIDIOC_DQBUF' for device '%s' failed: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
			result = -1;
		break;
		default:
			tst_resm(TINFO,
				"Unknown error in the man capture loop for device '%s'. Unknown error.\n",
				conf_device_name_cap);
			result = -1;
		break;
	}

	if(ret)
		goto err0;

	ret = stop_capturing();
	switch(ret) {
		case SUCCESS:
			result = 0;
		break;
		case ERROR_VIDIOC_STREAMOFF:
			tst_resm(TINFO,
				"IOCTL 'VIDIOC_STREAMOFF' for device '%s' failed: %d, %s.\n",
				conf_device_name_cap, errno, strerror(errno));
			result = -1;
		break;
		default:
			tst_resm(TINFO,
				"Could not start capturing from device '%s'. Unknown error.\n",
				conf_device_name_cap);
			result = -1;
		break;
	}

	if(ret)
		goto err0;

	return 0;
err0:
	return result;
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
	printf("  -i <device>         Select the input (capture) device (default: /dev/video0)\r\n");
	printf("  -o <device>         Select the output (framebuffer) device (default: /dev/fb0)\r\n");
	printf("  -m <device>         Select the I/O method out of (mmap|userptr|read)(default: mmmap)\r\n");
	printf("  -c <frame count>    Select frame count to capture. (default: 100)\r\n");
	printf("  -w <frame width>    Select frame width to capture. (default: 640)\r\n");
	printf("  -h <frame height>   Select frame height to capture. (default: 480)\r\n");
	printf("  -f <video format>   Select video format to capture. (default: YUV420)\r\n");
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
	int opt_i = 0, opt_o = 0, opt_m = 0, opt_c = 0, opt_w = 0, opt_h = 0, opt_f = 0, ret = 0;
	char *str_i, *str_o, *str_m, *str_c, *str_w, *str_h, *str_f, *msg;
	
	option_t options[] = {
		{"i:", &opt_i, &str_i},
		{"o:", &opt_o, &str_o},
		{"m:", &opt_m, &str_m},
		{"n:", &opt_c, &str_c},
		{"w:", &opt_w, &str_w},
		{"w:", &opt_f, &str_f},
		{"h:", &opt_h, &str_h},
		{NULL, NULL, NULL}
	};	

	/* parse options */
	if((msg = parse_opts(argc, argv, options, &help)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	if(opt_i)
		conf_device_name_cap = str_i;
		
	if(opt_o)
		conf_device_name_fb = str_o;
		
	if(opt_m) {
		if(0 == strcmp(str_m, "mmap")) {
			conf_io_method = IO_METHOD_MMAP;
		} else if(0 == strcmp(str_m, "userptr")) {
			conf_io_method = IO_METHOD_USERPTR;
		} else if(0 == strcmp(str_m, "read")) {
			conf_io_method = IO_METHOD_READ;
		} else {
			conf_io_method = IO_METHOD_MMAP;		
		}
	}
	
	if(opt_c) {
		if (sscanf(str_c, "%d", &conf_frame_count) != 1) {
			tst_resm(TWARN,
				"Could not get frame count. Using standard frame count of 100 now.");
		}
	}
		
	if(opt_w) {
		if (sscanf(str_w, "%d", &conf_frame_width) != 1) {
			tst_resm(TWARN,
				"Could not get frame width. Using standard frame count of 640 now.");
		}
	}

	if(opt_h) {
		if (sscanf(str_h, "%d", &conf_frame_height) != 1) {
			tst_resm(TWARN,
				"Could not get frame height. Using standard frame count of 480 now.");
		}
	}

	if(opt_f) {
		if(0 == strcmp(str_f, "YUV420")) {
			conf_pixelformat = V4L2_PIX_FMT_YUV420;
		/* TODO: Add more pixelformats. */
		} else {
			tst_resm(TWARN,
				"Could not get pixelformat. Using standard frame count of YUV420 now.");
			conf_io_method = V4L2_PIX_FMT_YUV420;		
		}
	}
	
	/* do initialization */
	setup();

	/* start the test */
	tst_resm(TINFO,
		"Starting video4linux2 capture test. (input: %s   output: %s)",
		conf_device_name_cap, conf_device_name_fb);
	
	/* perform the test */
	ret = test();
	
	/* evaluation */
	if (ret != 0)
		tst_resm(TFAIL, 
			"Capturing was not successful. Test failed.");
	else
		tst_resm(TPASS, 
			"Capturing was successful. Test passed.");
	
	cleanup();
	tst_exit();
}