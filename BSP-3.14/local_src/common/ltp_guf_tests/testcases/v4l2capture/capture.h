/******************************************************************************/
/* Copyright (C) 2011 Garz & Fricke GmbH						 			  */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        capture.h                                                     */
/*                                                                            */
/* Description: Header for the capture module.                            	  */
/*                                                                            */
/* Author:      Carsten Behling <carsten.behling@garz-fricke.com>             */
/*                                                                            */
/******************************************************************************/
#ifndef CAPTURE_H
#define CAPTURE_H

#include "v4l2capture.h"

extern char *conf_device_name_cap;
extern io_method_t conf_io_method;
extern unsigned int conf_frame_count;
extern unsigned int conf_frame_width;
extern unsigned int conf_frame_height;
extern unsigned int conf_pixelformat;

extern error_t open_capture_device(void);
extern error_t close_capture_device(void);
extern error_t init_capture_device(void);
extern error_t uninit_capture_device(void);
extern error_t start_capturing(void);
extern error_t stop_capturing(void);
extern error_t capture_mainloop(void);
#endif /* CAPTURE_H */