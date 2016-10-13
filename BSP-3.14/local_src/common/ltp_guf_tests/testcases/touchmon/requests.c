#include "touchmon.h"

int do_request_poll() {
	static int poll_counter = 0;
	int srv_socket_fd;

	struct req_message request;
	struct rep_message reply;

	int chars_sent = 0;

	srv_socket_fd = connect_to_server(setting_socket_poll);

	request.type = REQUEST_POLL;

	chars_sent = send(srv_socket_fd, (void *) &request, sizeof(struct req_message), 0);
	if(chars_sent == -1)
	{
		test_brk = TRUE;
		tst_brkm(TBROK, terminate, "Could not send poll-request! error:%i %s", errno, strerror(errno));
	}
	else if(chars_sent < sizeof(struct req_message)) {
		// Message not sent completely!
		test_brk = TRUE;
		tst_brkm(TBROK, terminate, "could not send complete request! bytes written:%i of %i", chars_sent, sizeof(struct req_message));
	}

	if(setting_verbose >= 3)
		fprintf(stderr, "Touchtest client [%d]: Request sent, waiting for reply\n", poll_counter);

	if(recvfrom(srv_socket_fd, (void *) &reply, sizeof(struct rep_message), 0, NULL, NULL) == -1) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate, "Failed to receive a poll-reply from the server: errno %i %s", errno, strerror(errno));
	}
	close(srv_socket_fd);
	if(setting_verbose >= 3)
		fprintf(stderr, "Touchtest client [%d]: ", poll_counter++);

	if(reply.type == REPLY_POLL) {
		if(setting_verbose >= 3)
			fprintf(stderr, "%s received\n", reply.value ? "'running'" : "'waiting'");
		return reply.value;
	}
	test_brk = TRUE;
	if(setting_verbose >= 3)
		fprintf(stderr, "invalid reply from server received\n");
	tst_brkm(TBROK, terminate, "Invalid reply from server!");
	exit(2);
}

/* send a request for the current countervalue to the socket specified in setting_socket_r */
int do_request_count() {
	int srv_socket_fd;

	struct req_message request;
	struct rep_message reply;

	int chars_sent = 0;

	srv_socket_fd = connect_to_server(setting_socket_r);

	request.type = REQUEST_COUNTERVAL;

	chars_sent = send(srv_socket_fd, (void *) &request, sizeof(struct req_message), 0);
	if(chars_sent == -1)
	{
		test_brk = TRUE;
		tst_brkm(TBROK, terminate, "Could not send request for countervalue! error:%i %s", errno, strerror(errno));
	}
	else if(chars_sent < sizeof(struct req_message)) {
		// Message not sent completely!
		test_brk = TRUE;
		tst_brkm(TBROK, terminate, "could not send complete request! bytes written:%i of %i", chars_sent, sizeof(struct req_message));
	}

	if(setting_verbose >= 3)
		fprintf(stderr, "Touchtest client: Request sent, waiting for reply\n");

	if(recvfrom(srv_socket_fd, (void *) &reply, sizeof(struct rep_message), 0, NULL, NULL) == -1) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate, "Failed to receive a poll-reply from the server: errno %i %s", errno, strerror(errno));
	}

	if(setting_verbose >= 3)
		fprintf(stderr, "Touchtest client: reply received\n");

	close(srv_socket_fd);

	if(reply.type == REPLY_COUNTERVAL) {
		return reply.value;
	}
	else {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate, "Invalid reply from server!");
		exit(2);
	}
}

/* send a request for counter-reset to the socket specified in setting_socket_r */
void do_request_reset() {
	int srv_socket_fd;

	struct req_message request;

	request.type = REQUEST_RESET;

	srv_socket_fd = connect_to_server(setting_socket_r);

	if(send(srv_socket_fd, (void *) &request, sizeof(struct req_message), 0) == -1)
	{
		tst_resm(TWARN, "Could not send reset request: errno %i %s\n", errno, strerror(errno));
	}

	close(srv_socket_fd);
}

/* send a request for termination to the socket specified in setting_socket_poll */
void do_request_term() {
	int srv_socket_fd;

	struct req_message request;

	request.type = REQUEST_TERM;

	srv_socket_fd = connect_to_server(setting_socket_r);

	if(send(srv_socket_fd, (void *) &request, sizeof(struct req_message), 0) == -1)
	{
		tst_resm(TWARN, "Could not send termination request! errno: %i %s\n", errno, strerror(errno));
	}

	close(srv_socket_fd);
}
