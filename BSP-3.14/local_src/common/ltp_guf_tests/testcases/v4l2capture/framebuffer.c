/******************************************************************************/
/* Copyright (C) 2011 Garz & Fricke GmbH						 			  */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        framebuffer.c                                                 */
/*                                                                            */
/* Description: Module for Linux framebuffer access.                          */
/*                                                                            */
/* Author:      Carsten Behling <carsten.behling@garz-fricke.com              */
/*                                                                            */
/******************************************************************************/

/* Standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

/* Local include files */
#include "v4l2capture.h"

/* Types*/
/* Structs */
/* Global variables */
char *conf_device_name_fb  = "/dev/fb0"; /* name of the standard
											framebuffer device*/
void *framebuffer = NULL; /* framebuffer memory */
int framebuffer_size = 0;
struct fb_fix_screeninfo fixed_info; /* fix screen info */
struct fb_var_screeninfo var_info; /* var screen info */
static int  fd_fb = -1;	/* file descriptor of the framebuffer device */

/* Local helper function declareations */

/* Global functions*/
/******************************************************************************/
/*                                                                            */
/* Function:    open_framebuffer_device                                       */
/*                                                                            */
/* Description: Checks and opens the framebuffer device.                      */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_STAT, if device node is not present.       */
/*                         - ERROR_ISCHR, if device isn't a character device. */
/*                         - ERROR_OPEN, if the open system call fails.       */
/*              On success - SUCCESS.                         	              */
/*                                                                            */
/******************************************************************************/
error_t open_framebuffer_device(void)
{
	error_t ret = SUCCESS;
    struct stat st;

    if (-1 == stat (conf_device_name_fb, &st)) {
		ret = ERROR_STAT;
		goto err0;
    }

    if (!S_ISCHR (st.st_mode)) {
		ret = ERROR_ISCHR;
		goto err0;
    }

    fd_fb = open(conf_device_name_fb, O_RDWR , 0);

    if (-1 == fd_fb) {
		ret = ERROR_OPEN;
		goto err0;
    }

	return SUCCESS;
err0:
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* Function:    close_framebuffer_device                                      */
/*                                                                            */
/* Description: Closes the framebuffer device.              	              */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_CLOSE, if the close system call fails.     */
/*              On success - SUCESS.                                          */
/*                                                                            */
/******************************************************************************/
error_t close_framebuffer_device(void)
{
	error_t ret = SUCCESS;
	if (close(fd_fb) == -1) {
		ret = ERROR_CLOSE;
		goto err0;
	}
	
	return SUCCESS;
err0:
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* Function:    init_framebuffer_device                                       */
/*                                                                            */
/* Description: Initialises the framebuffer device.              	          */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_FBIOGET_FSCREENINFO, if fix screen info    */
/*                                                      could not be          */
/*                                                      retrieved.            */
/*                         - FBIOGET_VSCREENINFO, if var screen info          */
/*                                                      could not be          */
/*                                                      retrieved.            */
/*                         - ERROR_MMAP, if framebuffer memory could not be   */
/*                                       mapped.                              */
/*              On success - SUCCESS.                                         */
/*                                                                            */
/******************************************************************************/
error_t init_framebuffer_device(void)
{
	error_t ret = SUCCESS;
    int ppc_fix = 0;
	long pagesize = getpagesize();

    if (ioctl(fd_fb, FBIOGET_FSCREENINFO, &fixed_info) < 0) {
		ret = ERROR_FBIOGET_FSCREENINFO;
		goto err0;
    }

   if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &var_info) < 0) {
		ret = ERROR_FBIOGET_VSCREENINFO;
		goto err0;
    }

    /* Now memory map the framebuffer.
       According to the SDL source code, it's necessary to
       compensate for a buggy mmap implementation on the
       PowerPC. This should not be a problem for other
       architectures. (This fix is lifted from SDL_fbvideo.c) */
    ppc_fix = (((long)fixed_info.smem_start) -
	       ((long) fixed_info.smem_start & ~(pagesize-1)));
    framebuffer_size = fixed_info.smem_len + ppc_fix;
    framebuffer = mmap(NULL,
		       framebuffer_size,
		       PROT_READ | PROT_WRITE,
		       MAP_SHARED,
		       fd_fb,
		       0);

    if (framebuffer == NULL) {
		ret = ERROR_MMAP;
		goto err0;
    }
	
	return SUCCESS;
err0:
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* Function:    uninit_framebuffer_device                                     */
/*                                                                            */
/* Description: Uninitialises the framebuffer device.              	          */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_MUNMAP, if framebuffer memory              */
/*                                         could not be unmaped.              */
/*              On success - SUCCESS.                                         */
/*                                                                            */
/******************************************************************************/
error_t uninit_framebuffer_device(void)
{
	error_t ret = SUCCESS;

	if (0 > munmap(framebuffer, framebuffer_size)) {
		ret = ERROR_MUNMAP;
	}

	return ret;
}

/******************************************************************************/
/*                                                                            */
/* Function:    switch_framebuffer                                            */
/*                                                                            */
/* Description: Switches the current framebuffer (double buffering).          */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_FBIOPAN_DISPLAY, if framebuffer could not  */
/*                                         be switched.  		              */
/*              On success - SUCCESS.                                         */
/*                                                                            */
/******************************************************************************/
error_t switch_framebuffer(unsigned int buffer)
{
	error_t ret = SUCCESS;
	var_info.activate = FB_ACTIVATE_VBL;
	if(0 == buffer) {
		var_info.yoffset = 0;
	} else if (1 == buffer){
		var_info.yoffset = 480;
	}
	if(ioctl(fd_fb, FBIOPAN_DISPLAY, &var_info)< 0) {
		ret = ERROR_FBIOPAN_DISPLAY;
		goto err0;
    }
	return SUCCESS;
err0:
	return ret;
}

/* Local helper functions */
