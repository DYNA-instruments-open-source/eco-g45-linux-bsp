/******************************************************************************/
/*                                                                            */
/* File:        format.c                                                      */
/*                                                                            */
/* Description: Module for video format conversion.                           */
/*                                                                            */
/* Author:      Carsten Behling <carsten.behling@garz-fricke.com              */
/*                                                                            */
/******************************************************************************/

/* Standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/time.h>

/* Local include files */
#include "v4l2capture.h"
#include "capture.h"
#include "framebuffer.h"

#ifdef NEON
#include <arm_neon.h>
#endif

/* local defines */
/* Structs */
/* Types */
/* Global variables */
/* Local helper function declarations */
inline unsigned int clamp(int x);

/* Global functions */
/******************************************************************************/
/*                                                                            */
/* Function:    format_yuv420_fb                                              */
/*                                                                            */
/* Description: Converts a YUV420 frame to the desired framebuffer format.    */
/*                                                                            */
/* Input:       capture_frame - The captured framebuffer.                     */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      None.                                                         */
/*                                                                            */
/******************************************************************************/
void format_yuv420_fb(const void *capture_frame) {
	int pos_x = 0;
	int pos_y = 0;
	
	char *in_frame = (char *) capture_frame;
	unsigned int p1 = 0;
	unsigned int p2 = 0;
	unsigned int p3 = 0;
	int y = 0;
	int u = 0;
	int v = 0;
	int r = 0;
	int g = 0;
	int b = 0;
		
	unsigned int pixel_value = 0;
	static int current_framebuffer = 1;
	//struct timeval start, stop; /* time measuring */
	
	int u_start = conf_frame_width * conf_frame_height;
	int v_start = u_start + (conf_frame_width * conf_frame_height / 4);

	//gettimeofday(&start, NULL);
	
	/* Check if the buffers are valid. */
	if((NULL == capture_frame) || (NULL == framebuffer)) {
		goto out;
	}
	
	for (pos_y = 0; pos_y < conf_frame_height; pos_y++)
	{
		for (pos_x = 0; pos_x < conf_frame_width; pos_x++)
		{
			p1 = (pos_y * conf_frame_width) + (pos_x);
			p2 = ((pos_y / 2) * conf_frame_width / 2) + (pos_x / 2) + u_start;
			p3 = ((pos_y / 2) * conf_frame_width / 2) + (pos_x / 2) + v_start;

			y = in_frame[p1] - 16; u = in_frame[p2] - 128; v = in_frame[p3] - 128;

			r = 298 * y + 0 * u + 409 * v;
			g = 298 * y - 100 * u - 208 * v;
			b = 298 * y + 516 * u + 0 * v;
			
			r = clamp((r + 128) >> 8);
			g = clamp((g + 128) >> 8);
			b = clamp((b + 128) >> 8);

			if (fixed_info.visual == FB_VISUAL_TRUECOLOR) {
				pixel_value = (((r >> (8 - var_info.red.length)) <<
				var_info.red.offset) |
				((g >> (8 - var_info.green.length)) <<
				var_info.green.offset) |
				((b >> (8 - var_info.blue.length)) <<
				var_info.blue.offset));
			} else {
				/* Pixel format not supported yet. */
				goto out;
			}


			switch (var_info.bits_per_pixel) {
				case 16:
					if(0 == current_framebuffer) {
						*((unsigned short *)framebuffer +
							fixed_info.line_length/2 * (pos_y + var_info.yres) + pos_x) =
							(unsigned short)pixel_value;
					} else {
						*((unsigned short *)framebuffer +
							fixed_info.line_length/2 * pos_y + pos_x) =
							(unsigned short)pixel_value;
					
					}
				break;
				default:
					/* Pixel depth not supported yet. */
					goto out;
				break;
			}			
		}
	}
	if(0 == current_framebuffer) current_framebuffer = 1;
	else if(1 == current_framebuffer) current_framebuffer = 0;
	switch_framebuffer(current_framebuffer);
	//gettimeofday(&stop, NULL);
	//printf("Elapsed: %ld ms\n", (unsigned long)((stop.tv_sec * 1000 + stop.tv_usec/1000) - (start.tv_sec * 1000 + start.tv_usec/1000)));
	return;

out:
	return;
}
/* Local helper functions */
/******************************************************************************/
/*                                                                            */
/* Function:    clamp                                                  */
/*                                                                            */
/* Description: Clamps the components within 8 bit range.                     */
/*                                                                            */
/* Input:       x - Value to clamp.                     					  */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      Clamped component.                                            */
/*                                                                            */
/******************************************************************************/
inline unsigned int clamp(int x)
{
	int r = x; /* round to nearest */
	if (r < 0)         return 0;
	else if (r > 255)  return 255;
	else               return r;
}
