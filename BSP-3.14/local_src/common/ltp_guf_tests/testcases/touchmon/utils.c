#include "touchmon.h"

/* helper functions */
int time_ms_diff(struct timeval *tv1, struct timeval *tv2) {
	return (tv2->tv_sec - tv1->tv_sec)*1000 + (tv2->tv_usec - tv1->tv_usec)/1000;
}

// calculate time difference between two given structs and save it into the third one
void time_diff(struct timeval *tv1, struct timeval *tv2, struct timeval *tvout) {
	long out_sec;
	long out_usec;

	if(tv1 != NULL && tv2 != NULL && tvout != NULL) {
		out_sec = tv2->tv_sec - tv1->tv_sec;
		out_usec = tv2->tv_usec - tv1->tv_usec;

		if(out_usec < 0) {
			out_usec += 1000000;
			out_sec -= 1;
		}

		tvout->tv_sec = out_sec;
		tvout->tv_usec = out_usec;
	}
}

int is_test_passed(int eventCnt) {
	if(setting_counter_bound_lower <= eventCnt && setting_counter_bound_upper >= eventCnt) {
		return TRUE;
	}

	return FALSE;
}

int connect_to_server(char* socket_path) {
	struct sockaddr_un srv_addr;
	int socket_fd;

	// connect
	if(setting_verbose >= 3)
		fprintf(stderr, "creating socket\n");

	// create socket => socket()
	if((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate, "could not create socket! error:%i %s", errno, strerror(errno));
		exit(2);
	}

	if(setting_verbose >= 3)
		fprintf(stderr, "socket created, connecting to server\n");


	srv_addr.sun_family = AF_UNIX;
	strncpy(&srv_addr.sun_path[0], socket_path, sizeof(srv_addr.sun_path) - 1);

	if(connect(socket_fd, (struct sockaddr*) &srv_addr, sizeof(struct sockaddr_un)) == -1) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate, "could not connect to server! error:%i %s", errno, strerror(errno));
		exit(2);
	}

	if(setting_verbose >= 3)
		fprintf(stderr, "connection established, sending request\n");

	return socket_fd;
}



/* send a request for the readyness of the server to the socket specified in setting_socket_r */

/* wait for the server-socket to show up, then poll the server until it signals readyness.*/
/* Returns TRUE on success, and FALSE on Timeout */
int do_poll_server(int timeout) {
	struct timeval time_begin, time_now;
	int socket_exists = FALSE;

	struct stat file_stat;

	gettimeofday(&time_begin, NULL);

	do {
		// check if file exists: -s or -r option
		if(stat(setting_socket_poll, &file_stat) == 0) {
			socket_exists = TRUE;
			break;
		}

		usleep(100000); // 100ms
		gettimeofday(&time_now, NULL);
	} while(timeout == 0 || time_ms_diff(&time_begin, &time_now) < timeout);

	if(! socket_exists) {
		return FALSE; // timeout
	}

	do {
		if(do_request_poll()) {
			return TRUE;
		}
		usleep(100000);	// 100ms
		gettimeofday(&time_now, NULL);
	} while(timeout == 0 || time_ms_diff(&time_begin, &time_now) < timeout);

	return FALSE; // timeout
}

