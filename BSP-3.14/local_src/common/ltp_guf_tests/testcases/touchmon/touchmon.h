#ifndef TOUCHMON_H
#define TOUCHMON_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include <sys/time.h>
#include <linux/input.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


/* harness specific includes */
#include "test.h"
#include "usctest.h"

#define max(x, y) ((x > y) ? (x) : (y))
#define min(x, y) ((x > y) ? (y) : (x))

#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif

/* default values: */
extern char *setting_inputdev;
extern char *setting_socket_r;
extern char *setting_socket_s;
extern char *setting_socket_poll;
extern char *setting_logfile;
extern int setting_counter_bound_lower;
extern int setting_counter_bound_upper;
extern int setting_timeout;
extern int setting_daemonize;
extern int setting_poll_server;
extern int setting_poll_timeout;
extern int setting_send_terminate;
extern int setting_send_reset;
extern int setting_verbose;
extern int setting_filter_mode_or;

/* global variables */
extern char *TCID;
extern int  TST_TOTAL;

extern int run;
extern int event_thread_ready;

extern struct filter_instance* filter_head;

/* global result variables */
extern int input_event_counter;	/* event counter for input_thread */
extern int reply_event_counter;	/* event counter for reply received in client-mode */

extern int test_brk;				/*	global variable to indicate that there is an unexpected 
									failure when terminating*/


/* filter structures */
struct named_filter {
	char *name;					// name of the filter (used in -f option)
	char *descr;				// description of the filter (displayed in help text)
	int num_params_max;
	int num_params_min;
	int (*filter_function) (struct input_event*, int*, int, void**);
			// function to determine if the event should be counted according to this filter
};

struct filter_instance {
	struct named_filter* filter;	// filter-type
	int* params;					// params[] => options passed on invocation
	int num_params;					// length of params[]
	struct filter_instance* next;	// next filter-instance in the linked-list of active filters
	void* state;					// filter specific state, may be set by the filter itself
};


/* Client/Server protocol: */
#define REQUEST_COUNTERVAL	1
#define REQUEST_TERM		2
#define REQUEST_RESET		3
#define REQUEST_POLL		4

#define REPLY_COUNTERVAL	1
#define REPLY_POLL			4

struct req_message {
	int type;
};

struct rep_message {
	int type;
	int value;
};

/* test functions */
void setup();
void cleanup(int return_code);
void terminate();
void help();

/* threads */
void *event_thread(void *ptr);
void *server_thread(void *ptr);

/* utils */
int time_ms_diff(struct timeval* tv1, struct timeval* tv2);
void time_diff(struct timeval *tv1, struct timeval *tv2, struct timeval *tvout);
int is_test_passed(int eventCnt);
int connect_to_server(char* socket_path);
int do_poll_server(int timeout);

/* filter */
int apply_filters(struct input_event* evt);
int instantiate_filter(char* arg);
void free_filters();
void print_filter_help();

/* requests */
int do_request_poll();
int do_request_count();
void do_request_reset();
void do_request_term();
#endif