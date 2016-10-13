#include "touchmon.h"

#ifndef FILTERS_H
#define FILTERS_H

/**
  * filters.c
  * This File lists all available event-filters.
  * To add your own filters, copy an existing filter_*.c and modify it to your needs.
  * Then add the structure therein to the following array. Additionally the sourcefile 
  * has to be added to $SRCS in the Makefile.
  */

extern struct named_filter filter_sync;
extern struct named_filter filter_raw;
extern struct named_filter filter_touchpress;
extern struct named_filter filter_toucharea;

struct named_filter* filters_installed[] = {
	&filter_sync,
	&filter_raw,
	&filter_touchpress,
	&filter_toucharea,
	NULL,
};

#endif