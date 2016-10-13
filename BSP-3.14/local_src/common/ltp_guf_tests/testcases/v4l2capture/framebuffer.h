/******************************************************************************/
/* Copyright (C) 2011 Garz & Fricke GmbH						 			  */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        framebuffer.h                                                 */
/*                                                                            */
/* Description: Header for the framebuffer module.                            */
/*                                                                            */
/* Author:      Carsten Behling <carsten.behling@garz-fricke.com>             */
/*                                                                            */
/******************************************************************************/
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "v4l2capture.h"

extern char *conf_device_name_fb;
extern void *framebuffer;
extern int framebuffer_size;
extern struct fb_fix_screeninfo fixed_info;
extern struct fb_var_screeninfo var_info;

extern error_t open_framebuffer_device(void);
extern error_t init_framebuffer_device(void);
extern error_t uninit_framebuffer_device(void);
extern error_t close_framebuffer_device(void);
extern error_t switch_framebuffer(unsigned int buffer);
#endif /* FRAMEBUFFER_H */