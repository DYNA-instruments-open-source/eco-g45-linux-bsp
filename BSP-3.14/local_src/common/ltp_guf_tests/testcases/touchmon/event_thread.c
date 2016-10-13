#include "touchmon.h"

/* Thread to read from input event device */
void *event_thread(void *ptr) {

	int event_fd;
	FILE *log_fd;
	struct input_event evt_buffer;

	fd_set fdset;

	struct timeval time_begin, time_now, timeout, time_done;

	if(setting_verbose >= 3)
		fprintf(stderr, "Touchtest Event: open touch device: %s\n", setting_inputdev);

	gettimeofday(&time_begin, NULL);
	event_fd = open(setting_inputdev, O_RDONLY);
	if(event_fd == -1) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate, "Could not open event device! Error: %i %s", errno, strerror(errno));
	}

	if(setting_verbose >= 3)
		fprintf(stderr, "Touchtest Event: opened touch device: %s [%d]\n", setting_inputdev, event_fd);

	if(setting_logfile != NULL) {
		if(setting_verbose >= 3)
			fprintf(stderr, "Touchtest Event: open log file: %s\n", setting_logfile);
		log_fd = fopen(setting_logfile, "a");
		if(log_fd == NULL) {
			tst_resm(TWARN, "Could not open logfile! Error:%i %s", errno, strerror(errno));
		}
	}

	if(setting_timeout > 0) {
		if(setting_verbose >= 3)
			fprintf(stderr, "Touchtest Event: timeout: %d\n", setting_timeout);
		time_done.tv_sec = time_begin.tv_sec + (setting_timeout/1000);
		time_done.tv_usec = time_begin.tv_usec + ((setting_timeout%1000) * 1000);

		if(time_done.tv_usec > 1000000) {
			time_done.tv_usec -= 1000000;
			time_done.tv_sec++;
		}
	}
	if(setting_verbose >= 3)
		fprintf(stderr, "Touchtest Event: Starting to read events\n");

	event_thread_ready = TRUE;

	while(run) {
		if(setting_timeout > 0) {
			gettimeofday(&time_now, NULL);
			if(time_ms_diff(&time_begin, &time_now) >= setting_timeout) {
				run = FALSE;
				break;
			}
			else {
				time_diff(&time_now, &time_done, &timeout);
			}
		}
		else {
			timeout.tv_sec = 0;
			timeout.tv_usec = 10000;	// sleep max 10ms, to check if we should terminate
		}

		FD_ZERO(&fdset);
		FD_SET(event_fd, &fdset);

		if(select(event_fd + 1, &fdset, NULL, NULL, &timeout) == 1) {
			// data to read:
			if(read(event_fd, (void *) &evt_buffer, sizeof(struct input_event)) != sizeof(struct input_event)) {
				if(setting_verbose >= 3)
					fprintf(stderr, "event_read: read was incomplete or had an error!");

				if(setting_logfile != NULL && log_fd != NULL)
					fprintf(log_fd, "error => read was incomplete or had an error!");
			}

			if(setting_verbose >= 3)
				fprintf(stderr, "event => timestamp: %li type: 0x%x code: 0x%x value: %d\n", evt_buffer.time.tv_sec * 1, evt_buffer.type, evt_buffer.code, evt_buffer.value);

			if(setting_logfile != NULL && log_fd != NULL)
				fprintf(log_fd, "event => timestamp: %li type: 0x%x code: 0x%x value: %d\n", evt_buffer.time.tv_sec * 1, evt_buffer.type, evt_buffer.code, evt_buffer.value);

			if(apply_filters(&evt_buffer))
				input_event_counter++;
		}
	}

	event_thread_ready = FALSE;

	if(setting_logfile != NULL && log_fd != NULL)
		fclose(log_fd);

	close(event_fd);

	return NULL;
}
