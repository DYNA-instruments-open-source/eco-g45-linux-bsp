/******************************************************************************/
/* Copyright (C) 2011 Garz & Fricke GmbH						 			  */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        format.h                                                      */
/*                                                                            */
/* Description: Header for the format coversion.                          	  */
/*                                                                            */
/* Author:      Carsten Behling <carsten.behling@garz-fricke.com >            */
/*                                                                            */
/******************************************************************************/
#ifndef FORMAT_H
#define FORMAT_H

#include "v4l2capture.h"
extern void format_yuv420_fb(const void *capture_frame);
#endif /* FORMAT_H */