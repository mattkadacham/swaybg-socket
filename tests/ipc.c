#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "ipc.h"

int main(void) {
	char socket_path[128];
	snprintf(socket_path, sizeof(socket_path), "/tmp/swaybg-ipc-test-%ld.sock",
		(long)getpid());
	int server = ipc_open_server(socket_path);
	if (server == -1) {
		return EXIT_FAILURE;
	}

	pid_t child = fork();
	if (child == 0) {
		char *args[] = {
			"cache", "forest", "/tmp/image with spaces.png", "fill",
		};
		char *response = NULL;
		int result = ipc_send_command(socket_path, 4, args, &response);
		bool failed = result != 0 || !response ||
			strcmp(response, "accepted") != 0;
		free(response);
		_exit(failed ? EXIT_FAILURE : EXIT_SUCCESS);
	}
	if (child == -1) {
		ipc_close_server(server, socket_path);
		return EXIT_FAILURE;
	}

	struct ipc_message message;
	bool failed = ipc_receive_message(server, &message) != 0;
	if (!failed) {
		failed = strcmp(ipc_message_arg(&message, 0), "cache") != 0 ||
			strcmp(ipc_message_arg(&message, 1), "forest") != 0 ||
			strcmp(ipc_message_arg(&message, 2),
				"/tmp/image with spaces.png") != 0 ||
			strcmp(ipc_message_arg(&message, 3), "fill") != 0 ||
			ipc_message_arg(&message, 4) != NULL;
		ipc_reply(&message, !failed, failed ? "rejected" : "accepted");
	}

	int status = 0;
	waitpid(child, &status, 0);
	ipc_close_server(server, socket_path);
	if (failed || !WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
