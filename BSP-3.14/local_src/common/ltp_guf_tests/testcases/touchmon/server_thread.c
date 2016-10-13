#include "touchmon.h"

/* server thread to serve requests comming from socket */
void *server_thread(void *ptr) {
	int srv_socket_fd, con_socket_fd;
	struct sockaddr_un srv_sock_addr;
	
	fd_set fdset;
	struct timeval timeout;
	
	struct req_message request;
	struct rep_message reply;

	if((srv_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate, "Server: could not create socket! error:%i %s", errno, strerror(errno));
	}
	
	srv_sock_addr.sun_family = AF_UNIX;
	strncpy(&srv_sock_addr.sun_path[0], setting_socket_s, sizeof(srv_sock_addr.sun_path) - 1);
	
	if(bind(srv_socket_fd, (struct sockaddr *) &srv_sock_addr, sizeof(struct sockaddr_un)) == -1) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate, "Server: could not bind to socket! error:%i %s", errno, strerror(errno));
	}
	
	if(listen(srv_socket_fd, 10) == -1) {
		test_brk = TRUE;
		tst_brkm(TBROK, terminate, "Server: could not listen on socket! error:%i %s", errno, strerror(errno));
	}
	
	while(run) {
		timeout.tv_sec = 0;
		timeout.tv_usec = 500000;

		if(setting_verbose >= 4)
			fprintf(stderr, "Touchtest server: waiting for clients\n");

		FD_ZERO(&fdset);
		FD_SET(srv_socket_fd, &fdset);
		
		if(select(srv_socket_fd + 1, &fdset, NULL, NULL, &timeout) == 1) {
			if(setting_verbose >= 3)
				fprintf(stderr, "Touchtest server: received request\n");
			// we have data to read:
			con_socket_fd = accept(srv_socket_fd, NULL, NULL);
			if(con_socket_fd == -1) {
				tst_resm(TWARN, "error while accepting request: errno %i %s", errno, strerror(errno));
				continue;
			}
			
			if(recv(con_socket_fd, (void *) &request, sizeof(struct req_message), 0) == -1) {
				tst_resm(TWARN, "error while receiving request: %i %s", errno, strerror(errno));
				close(con_socket_fd);
				continue;
			}
			
			// handle request:
			if(setting_verbose >= 3)
				fprintf(stderr, "Touchtest server: read request\n");	
			
			switch (request.type) {
				case REQUEST_COUNTERVAL:
					if(setting_verbose >= 2)
						fprintf(stderr, "Touchtest server: Handled counter request\n");	
				
					reply.type = REPLY_COUNTERVAL;
					reply.value = input_event_counter;
					if(send(con_socket_fd, (void *) &reply, sizeof(struct rep_message), 0) == -1)
					{
						tst_resm(TWARN, "Touchtest server: Could not send reply to request for countervalue: errno %i %s\n", errno, strerror(errno));
					}
					
					if(setting_verbose >= 2)
						fprintf(stderr, "Touchtest server: Handled request for countervalue\n");
					break;
				case REQUEST_POLL:
					if(setting_verbose >= 2)
						fprintf(stderr, "Touchtest server: Received poll-request\n");
					
					reply.type = REPLY_POLL;
					reply.value = event_thread_ready;
					if(send(con_socket_fd, (void *) &reply, sizeof(struct rep_message), 0) == -1)
					{
						tst_resm(TWARN, "Touchtest server: Could not send reply to poll-request: %i %s\n", errno, strerror(errno));
						break;
					}
					
					if(setting_verbose >= 2)
						fprintf(stderr, "Touchtest server: Handled poll-request\n");
						
					break;
				case REQUEST_TERM:
					run = FALSE;
					if(setting_verbose >= 2)
						fprintf(stderr, "Touchtest server: Received request for termination\n");
					break;
				case REQUEST_RESET:
					input_event_counter = 0;
					if(setting_verbose >= 2)
						fprintf(stderr, "Touchtest server: Received request for counter reset\n");
						
					break;
				default:
					if(setting_verbose >= 2) 
						fprintf(stderr, "Touchtest server: Received unsupported request!\n");
					break;
			}
			close(con_socket_fd);
		}
	}
	
	close(srv_socket_fd);
	unlink(setting_socket_s);
	return NULL;
}
