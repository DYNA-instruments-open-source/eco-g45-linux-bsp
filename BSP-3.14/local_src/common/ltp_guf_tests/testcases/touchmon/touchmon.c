/******************************************************************************/
/* Copyright (C) 2011 Garz&Fricke GmbH						 			      */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        touchmon.c                                                    */
/*                                                                            */
/* Description: This is a test for the Touchscreen.                           */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   touch                                                         */
/*                                                                            */
/* Test Assertion                                                             */
/* & Strategy:  A server process runs in the background while other tests     */
/*              execute. It counts input events from the touchscreen and      */
/*              provides access to them via a UNIX domain socket.             */
/*              The touch testcase itself will then contact the server and    */
/*              request event statistics.                                     */
/*                                                                            */
/* Author:      Hendrik Woidt <hendrik.woidt@garz-fricke.com                  */
/*                                                                            */
/******************************************************************************/

#include "touchmon.h"

/* default values: */
char *setting_inputdev = NULL;
char *setting_socket_r = NULL;
char *setting_socket_s = NULL;
char *setting_socket_poll = NULL;
char *setting_logfile = NULL;
int setting_counter_bound_lower = INT_MAX;
int setting_counter_bound_upper = 0;
int setting_timeout = 0;
int setting_daemonize = FALSE;
int setting_poll_server = 0;
int setting_poll_timeout = 0;
int setting_send_terminate = FALSE;
int setting_send_reset = FALSE;
int setting_verbose = 1;
int setting_filter_mode_or = FALSE;

/* global variables */
char *TCID     = "touchmon       ";	/* test program identifier.              */
int  TST_TOTAL = 1;					/* total number of tests in this file.   */

int run = TRUE;					/* global flag for running threads */
int event_thread_ready = FALSE;	/* global flag for readyness */

struct filter_instance* filter_head = NULL;		/* list head of the filter chain */

/* global result variables */
int input_event_counter = 0;	/* event counter for input_thread */
int reply_event_counter = 0;	/* event counter for reply received in client-mode */

int test_brk = 0;				/*	global variable to indicate that there is an unexpected
									failure when terminating*/


/******************************************************************************/
/*                                                                            */
/* Function:    cleanup                                                       */
/*                                                                            */
/* Description: Performs all one time clean up for this test on successful    */
/*              completion,  premature exit or  failure. Closes all temporary */
/*              files, removes all temporary directories exits the test with  */
/*              appropriate return code.                                      */
/*                                                                            */
/* Input:       Return code to exit with.                                     */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      Exits with the specified return code.                         */
/*                                                                            */
/******************************************************************************/
void cleanup(int ret) {
	free_filters();

	if(setting_socket_s != NULL) {
		unlink(setting_socket_s);
	}

	exit(ret);
}

/******************************************************************************/
/*                                                                            */
/* Function:    terminate                                                     */
/*                                                                            */
/* Description: Check whether the test has been passed or not and print a     */
/*              test summary.                                                 */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      Test summary according to verboseness.                        */
/*                                                                            */
/* Return:      On failure - Exits calling cleanup(). With '1' return code.   */
/*              On success - Exits calling cleanup(). With '0' return code.   */
/*                                                                            */
/******************************************************************************/
void terminate() {
	int return_code;

	if(test_brk == TRUE) {
		return_code = 2;
	}
	else if(setting_daemonize || setting_send_terminate) {
		return_code = 0;
	}
	else if(is_test_passed(max(input_event_counter, reply_event_counter))) {
		tst_resm(TPASS, "Number of received touch events(%i) is okay. Test passed.", max(input_event_counter, reply_event_counter));
		return_code = 0;
	}
	else {
		tst_resm(TFAIL, "There where %i touch events registered. Test failed.", max(input_event_counter, reply_event_counter));
		return_code = 1;
	}

	cleanup(return_code);
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
void help() {
	printf("** Touchmon\n");
	printf("Test programm for input-events. This programm listens on event input devices\n");
	printf("and exports event metrics through a socket. Alternatively it requests these metrics\n");
	printf("through a socket and determines whether the test has passed or failed.\n");
	printf("In case that there is a socket specified as source as well as an input device, the higher event\n");
	printf("count of the two will be used to determine the test result\n\n");

	printf("** Options:\n");
	printf("  -i dev   Start event listener on the specified input device. This is usually one of the devices \n");
	printf("           in /dev/input/eventN\n");
	printf("  -s sock  Start metrics server on the specified socket.\n");
	printf("  -r sock  Request metrics on specified socket. (has no effect when -s is specified)\n");
	printf("  -l file  Log events to the specified file (only when -i option is active)\n\n");

	printf("  -o time  Specify a timeout in ms to listen for input (input source is specified either via -r or -i).\n");
	printf("           0 means no timeout (default)\n\n");

	printf("  -c count Specify a count for the event counter to be reached in order to pass the test.\n");
	printf("           You can give this option multiple times. The test is passed, when the event counter \n");
	printf("           is in the range from the lowest provided number to the highest. Example: '-c 4' => pass\n");
	printf("           when event counter equal to 4. Another Example: '-c 10 -c 0' => pass when the counter is \n");
	printf("           less or equal than 10.\n\n");

	printf("  -d       Daemonize (only makes sense when starting a socket-server)\n");
	printf("  -z       Request the server to reset the event counter to zero, after receiving metric\n");
	printf("  -t       Request the server to terminate, after receiving metrics. (no effect, unless a socket is \n");
	printf("           specified via -r)\n");
	printf("  -p time  Poll the server for a maximum of <time> ms (0 means no timeout). This will wait until the \n");
	printf("           socket is reachable and will then \n");
	printf("           poll the server until it has completed setup (=> event thread is ready to receive events and\n");
	printf("           server is ready to serve requests).\n");
	printf("           If the -r option is present, the corresponding socket will be polled. Other requests \n");
	printf("           (e.g. -r, -z or -t) will be sent after that. If a daemonized server is started, (combination \n");
	printf("           of -d and -s), the parent will block until the server is ready\n\n");

	printf("  -f args  If the -i option has been specified, the events received from the device are filtered acording \n");
	printf("           to the filter specified in args. The -f option may be specified multiple times. Normally an \n");
	printf("           event is counted only if it passes all specified filters. If you specify '-f mode_or', an event\n");
	printf("           is counted if it passes at least one of the specified filters. For info about installed filters\n");
	printf("           and their arguments see 'Filters' below\n\n");

	printf("  -v       Be verbose. This will print out a status message for each request which was handled by the server\n");
	printf("  -vv      Be really verbose. This will print out debug messages on stderr to trace what the programm currently does\n");
	printf("  -vvv     Be extremely verbose. This will make the event thread write event-data to stderr for each received event\n");
	printf("  -q       Be quiet. Only the test's pass/fail message is printed\n");
	printf("  -h       Display this help message\n\n");

	printf("** Examples\n");
	printf("A use case for EMI tests would be:\n");
	printf("  First start the server to monitor the incomming events:\n");
	printf("    $ touchmon -d -s /touch_srv_socket -i /dev/input/event0\n");
	printf("  Then go on with the other tests. At the end of each session, check whether there where events\n");
	printf("  and reset the counter of the server:\n");
	printf("    $ touchmon -z -r /touch_srv_socket\n");
	printf("  Finally terminate the server:\n");
	printf("    $ touchmon -t -r /touch_srv_socket\n");
	printf("Note: the socket can be placed anywhere, as long as both processes have write access to it\n");
	printf("It is also possible to directly listen for events, without running a server:\n");
	printf("    $ touchmon -i /dev/input/event0 -o 60000 -c 50 -c 100\n");
	printf("  This will listen for one minute and pass the test if there are 50 to 100 events received\n\n");

	printf("** Filters\n");
	printf("Filters provide a flexible way to monitor the incomming events. To instantiate a Filter use the \n");
	printf("-f option with an argument conforming to the following syntax:\n");
	printf("  -f <FilterName>,<param1>,<param2>,<...>\n");
	printf("where <FilterName> is one of the installed filters, and <paramN> is a filter-specific parameter\n");
	printf("The following filters are installed:\n\n");

	print_filter_help();
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
/*               -i dev   Start event listener on the specified input device. */
/*               -s sock  Start metrics server on the specified socket.       */
/*               -r sock  Request metrics on specified socket. (has no effect */
/*                        when -s is specified)                               */
/*               -l file  Log events to the specified file (only when -i      */
/*                        option is active)                                   */
/*               -o time  Specify a timeout in ms to listen for input (input  */
/*                        source is specified either via -r or -i). 0 means no*/
/*                        timeout (default)                                   */
/*               -c count Specify a count for the event counter to be reached */
/*                        in order to pass the test. You can give this option */
/*                        multiple times. The test is passed, when the event  */
/*                        counter is in the range from the lowest provided    */
/*                        number to the highest. Example: '-c 4' => pass when */
/*                        event counter equal to 4.                           */
/*                        Another Example: '-c 10 -c 0' => pass when the      */
/*                        counter is less or equal than 10                    */
/*               -d       Daemonize (only makes sense when starting a         */
/*                        socket-server)                                      */
/*               -z       Request the server to set the event counter to zero */
/*                        (no effect, unless a socket is specified via -r)    */
/*               -t       Request the server to terminate, after receiving    */
/*                        metrics. (no effect, unless a socket is specified   */
/*                        via -r)                                             */
/*               -p time  Poll the server for a maximum of <time> ms (0 means */
/*                        no timeout). This will wait until the socket is     */
/*                        reachable and will then poll the server until it    */
/*                        has completed setup (=> event thread is ready to    */
/*                        receive events and server is ready to serve         */
/*                        requests). If the -r option is present, the         */
/*                        corresponding socket will be polled. Other requests */
/*                        (e.g. -r, -z or -t) will be sent after that. If a   */
/*                        daemonized server is started, (combination of -d    */
/*                        and -s), the parent will block until the server is  */
/*                        ready.                                              */
/*               -v       Be verbose. This will print out a status message for*/
/*                        each request which was handled by the server.       */
/*               -vv      Be really verbose. Will print out debug messages    */
/*                        on stderr to trace what the programm currently does.*/
/*               -vvv     Be extremely verbose. This will make the event      */
/*                        thread write event-data to stderr for each          */
/*                        received event                                      */
/*               -q       Be quiet. Only the test's result message is printed */
/*               -h       Display a Help message with usage information       */
/*                                                                            */
/* Exit:       On error   - exits with 2 exit value.                          */
/*             On failure - exits with 1 exit value.                          */
/*             On success - exits with 0 exit value.                          */
/*                                                                            */
/******************************************************************************/
int main(int argc, char **argv) {
	int opt;
	int tmp = 0;
	char* tmp_str = NULL;
	int child_pid;

	pthread_t thread_srv, thread_event;

	if(setting_verbose >= 3)
		fprintf(stderr, "starting command parsing\n");

	/* parse commandline options: */
	while ((opt = getopt(argc, argv, "i:s:r:l:p:o:c:f:dtzhvq")) != -1) {
		switch (opt) {
			case 'i':
				tmp_str = realloc(setting_inputdev, sizeof(char) * (strlen(optarg) + 1));
				if(tmp_str == NULL) {
					test_brk = TRUE;
					tst_brkm(TBROK, terminate, "Could not allocate memory for settings!");
				}
				setting_inputdev = tmp_str;
				strcpy(setting_inputdev, optarg);
				break;
			case 's':
				tmp_str = realloc(setting_socket_s, sizeof(char) * (strlen(optarg) + 1));
				if(tmp_str == NULL) {
					test_brk = TRUE;
					tst_brkm(TBROK, terminate, "Could not allocate memory for settings!");
				}
				setting_socket_s = tmp_str;
				strcpy(setting_socket_s, optarg);
				break;
			case 'r':
				tmp_str = realloc(setting_socket_r, sizeof(char) * (strlen(optarg) + 1));
				if(tmp_str == NULL) {
					test_brk = TRUE;
					tst_brkm(TBROK, terminate, "Could not allocate memory for settings!");
				}
				setting_socket_r = tmp_str;
				strcpy(setting_socket_r, optarg);
				break;
			case 'l':
				tmp_str = realloc(setting_logfile, sizeof(char) * (strlen(optarg) + 1));
				if(tmp_str == NULL) {
					test_brk = TRUE;
					tst_brkm(TBROK, terminate, "Could not allocate memory for settings!");
				}
				setting_logfile = tmp_str;
				strcpy(setting_logfile, optarg);
				break;
			case 'o':
				setting_timeout = atoi(optarg);
				break;
			case 'f':
				tmp = instantiate_filter(optarg);
				if(tmp != 0) {
					test_brk = TRUE;
					if(tmp == -1)
						tst_brkm(TBROK, terminate, "Could not instantiate the specified eventfilter: %s. This filter is not available. See help (-h option) for installed filters", optarg);
					else
						tst_brkm(TBROK, terminate, "Could not instantiate the specified eventfilter: %s. Wrong parameter count! See help (-h option) for information about installed filters", optarg);
				}
				break;
			case 'c':
				tmp = atoi(optarg);
				if(tmp > setting_counter_bound_upper)
					setting_counter_bound_upper = tmp;
				if(tmp < setting_counter_bound_lower)
					setting_counter_bound_lower = tmp;
				break;
			case 'p':
				setting_poll_server = TRUE;
				setting_poll_timeout = atoi(optarg);
				break;
			case 'd':
				setting_daemonize = TRUE;
				break;
			case 't':
				setting_send_terminate = TRUE;
				break;
			case 'z':
				setting_send_reset = TRUE;
				break;
			case 'v':
				setting_verbose++;
				break;
			case 'q':
				setting_verbose--;
				break;
			case 'h':
				help();
				cleanup(1);
				break;
			default:
				help();
				cleanup(1);
				break;
		}
	}

	if(setting_verbose >= 3)
		fprintf(stderr, "command parsing done, checking parameters\n");

	if(setting_counter_bound_lower > setting_counter_bound_upper) {
		// counter bounds still have defaults => pass only when no events are recorded
		setting_counter_bound_lower = 0;
		setting_counter_bound_upper = 0;
	}

	if(setting_poll_server) {
		if(setting_socket_s != NULL && setting_daemonize) {
			setting_socket_poll = setting_socket_s;
		}
		else if(setting_socket_r != NULL) {
			setting_socket_poll = setting_socket_r;
		}
		else {
			test_brk = TRUE;
			tst_brkm(TBROK, terminate, "There is no target specified for polling!");
		}
	}

	if(setting_verbose > 0) {
	/* check for warning and fail conditions*/
		/* check whether there is an input specified */
		if(setting_socket_r == NULL && setting_inputdev == NULL) {
			tst_resm(TWARN, "There is no input facility (either touch-device or a server socket) specified. There won't be any events registered");
		}
		/* check whether there is an output specified */
		if(setting_daemonize && setting_socket_s == NULL && setting_logfile == NULL) {
			tst_resm(TWARN, "There is no output (either server or logfile) specified. Since the programm will fork into background, you will get no output!");
		}
		/* check for missing termination condition*/
		if(setting_socket_r == NULL && setting_socket_s == NULL && setting_timeout == 0) {
			if(setting_daemonize) {
				test_brk = TRUE;
				tst_brkm(TBROK, terminate, "The programm would run indefinitely, without the server being able to receive a close request or a timeout. Since daemonizing would let the programm escape from your control, the test will be aborted");
			}
			else {
				tst_resm(TWARN, "Since you have neither specified to run a server nor gave a timeout, the programm will run until it is terminated by user-intervention!");
			}
		}
	}

	if(setting_verbose >= 3)
		fprintf(stderr, "checking done, doing setup\n");

	if(setting_verbose > 0) {
		/* if requested: print test-begin summary */
		if(setting_socket_s != NULL) {
			tst_resm(TINFO, "Starting Touchtest server on Socket %s, reading events from %s, timing out after %i ms %s",
										setting_socket_s,
										setting_inputdev,
										setting_timeout,
										setting_timeout ? "" : "(no timeout)"
					);
		}
		else {
			tst_resm(TINFO, "Starting Touchtest: %s from %s, timing out after %ims%s",
										(setting_inputdev != NULL) ? "reading events" : "requesting metrics",
										(setting_inputdev != NULL) ? setting_inputdev : setting_socket_r,
										setting_timeout,
										setting_timeout ? "" : "(no timeout)"
					);

			if(setting_send_terminate)
				tst_resm(TINFO, "Server will be terminated after requesting metrics");
			else if(setting_send_reset)
				tst_resm(TINFO, "Server will be reset after requesting metrics");

			tst_resm(TINFO, "Valid event count must be in the range of %i to %i", setting_counter_bound_lower, setting_counter_bound_upper);
		}
	}

	if(setting_daemonize) {
		child_pid = fork();
		if(child_pid > 0) {
			// we are parent; PID of child is child_pid
			if(setting_poll_server) {
				if(!do_poll_server(setting_poll_timeout)) {
					// kill child_pid here?
					tst_brkm(TBROK, terminate, " Timeout! The server did not manage to reply to poll request.");
				}
			}
			if(setting_verbose >= 3)
				fprintf(stderr, "Closing parent process thread.\n");
			exit(0);	// do not cleanup() since that would unlink the socket
		}
		else if(child_pid < 0) {
			// error
			tst_brkm(TBROK, terminate, "Failed to daemonize! Error: %i %s", errno, strerror(errno));
		}

		// now we are the child process
		setsid();

		chdir("/");

		setting_poll_server = FALSE;

		if(setting_verbose >= 3)
			fprintf(stderr, "successfully daemonized\n");

	}
	/* start server thread */
	if(setting_socket_s != NULL) {
		if(setting_verbose >= 3)
			fprintf(stderr, "starting server thread\n");

		pthread_create(&thread_srv, NULL, server_thread, NULL);
	}

	/* start input thread */
	if(setting_inputdev != NULL) {
		if(setting_verbose >= 3)
			fprintf(stderr, "starting event listener thread\n");

		pthread_create(&thread_event, NULL, event_thread, NULL);
	}

	/* poll server if(-r or (-s and -d))*/
	if(setting_poll_server && (setting_socket_r != NULL || (setting_socket_s != NULL && setting_daemonize))) {
		if(setting_verbose >= 3)
			fprintf(stderr, "Polling server until it is ready\n");

		if(!do_poll_server(setting_poll_timeout)) {
			test_brk = TRUE;
			tst_brkm(TBROK, terminate, "Timed out, while polling the Server!");
		}
	}

	/* get event count from server */
	if(setting_socket_r != NULL && setting_socket_s == NULL) {
		if(setting_verbose >= 3)
			fprintf(stderr, "requesting counter value from server\n");

		reply_event_counter = do_request_count();
	}

	/* request server to reset countervalue */
	if(setting_send_reset && setting_socket_r != NULL) {
		if(setting_verbose >= 3)
			fprintf(stderr, "requesting server to reset\n");

		do_request_reset();
	}

	/* request server to terminate */
	if(setting_send_terminate && setting_socket_r != NULL) {
		if(setting_verbose >= 3)
			fprintf(stderr, "requesting server to terminate\n");

		do_request_term();
	}

	if(setting_verbose >= 3)
		fprintf(stderr, "waiting for threads to finish\n");

	/* wait for running threads to close */
	if(setting_socket_s != NULL)
		pthread_join(thread_srv, NULL);
	if(setting_inputdev != NULL)
		pthread_join(thread_event, NULL);

	if(setting_verbose >= 3)
		fprintf(stderr, "terminating\n");

	/* print test summary and exit */
	terminate();

	if(setting_verbose >= 3)
		fprintf(stderr, "terminate did not exit... somthing is really wrong!\n");

	return 2;
}
