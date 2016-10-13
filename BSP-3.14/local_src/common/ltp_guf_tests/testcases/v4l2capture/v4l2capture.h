/******************************************************************************/
/* Copyright (C) 2011 Garz & Fricke GmbH						 			  */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        v4lcapture.h                                                  */
/*                                                                            */
/* Description: Common header for v4l2capture test.                           */
/*                                                                            */
/* Author:      Carsten Behling <carsten.behling@garz-fricke.com >            */
/*                                                                            */
/******************************************************************************/

#ifndef V4L2CAPTURE_H
#define V4L2CAPTURE_H

/* Common macros */
#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif

#define CLEAR(x) memset (&(x), 0, sizeof (x))

/* Types */
typedef enum {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
} io_method_t;

typedef enum {
    SUCCESS,
	AGAIN,
    ERROR_OPEN,
    ERROR_CLOSE,
	ERROR_MMAP,
	ERROR_MUNMAP,
	ERROR_SELECT,
	ERROR_READ,
    ERROR_STAT,
    ERROR_ISCHR,
	ERROR_VIDIOC_QUERYCAP,
	ERROR_VIDIOC_CROPCAP,
	ERROR_VIDIOC_S_CROP,
	ERROR_VIDIOC_S_FMT,
	ERROR_VIDIOC_REQBUFS,
	ERROR_VIDIOC_QUERYBUF,
	ERROR_VIDIOC_QBUF,
	ERROR_VIDIOC_DQBUF,
	ERROR_VIDIOC_STREAMON,
	ERROR_VIDIOC_STREAMOFF,
	ERROR_VID_NOT_A_CAP_DEV,
	ERROR_VID_NOT_IO_READ,
	ERROR_VID_NOT_IO_STREAM,
	ERROR_VID_NOT_ENOUGH_IO_BUF,
	ERROR_FBIOGET_FSCREENINFO,
	ERROR_FBIOGET_VSCREENINFO,
	ERROR_FBIOPAN_DISPLAY,
	ERROR_CALLOC,
	ERROR_MALLOC,
	ERROR_MEMALIGN,
	ERROR_TIMEOUT,
} error_t;
#endif /* V4L2CAPTURE_H */
