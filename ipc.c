#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include "ipc.h"

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

static bool set_socket_address(struct sockaddr_un *addr, const char *path) {
	if (strlen(path) >= sizeof(addr->sun_path)) {
		fprintf(stderr, "Socket path is too long: %s\n", path);
		return false;
	}

	memset(addr, 0, sizeof(*addr));
	addr->sun_family = AF_UNIX;
	strcpy(addr->sun_path, path);
	return true;
}

int ipc_open_server(const char *path) {
	struct sockaddr_un addr;
	if (!set_socket_address(&addr, path)) {
		return -1;
	}

	int fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (fd == -1) {
		fprintf(stderr, "Unable to create IPC socket: %s\n", strerror(errno));
		return -1;
	}

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		fprintf(stderr, "Unable to bind IPC socket %s: %s\n", path,
			strerror(errno));
		close(fd);
		return -1;
	}
	if (chmod(path, 0600) == -1 || listen(fd, 1) == -1) {
		fprintf(stderr, "Unable to initialize IPC socket %s: %s\n", path,
			strerror(errno));
		close(fd);
		unlink(path);
		return -1;
	}
	return fd;
}

int ipc_receive_message(int server_fd, struct ipc_message *message) {
	message->client_fd = accept(server_fd, NULL, NULL);
	message->data = NULL;
	message->size = 0;
	if (message->client_fd == -1) {
		fprintf(stderr, "Unable to accept IPC connection: %s\n", strerror(errno));
		return -1;
	}

	struct pollfd pfd = { .fd = message->client_fd, .events = POLLIN };
	if (poll(&pfd, 1, 1000) != 1) {
		fprintf(stderr, "IPC client did not send a command\n");
		close(message->client_fd);
		message->client_fd = -1;
		return -1;
	}

	message->data = malloc(IPC_MESSAGE_MAX);
	if (!message->data) {
		close(message->client_fd);
		message->client_fd = -1;
		return -1;
	}
	ssize_t size = recv(message->client_fd, message->data,
		IPC_MESSAGE_MAX, 0);
	if (size <= 0) {
		fprintf(stderr, "Unable to receive IPC command: %s\n",
			size == 0 ? "empty message" : strerror(errno));
		close(message->client_fd);
		free(message->data);
		message->client_fd = -1;
		message->data = NULL;
		return -1;
	}
	message->size = size;
	return 0;
}

const char *ipc_message_arg(const struct ipc_message *message, size_t index) {
	size_t offset = 0;
	for (size_t i = 0; offset < message->size; ++i) {
		const char *arg = message->data + offset;
		size_t remaining = message->size - offset;
		const char *end = memchr(arg, '\0', remaining);
		if (!end) {
			return NULL;
		}
		if (i == index) {
			return arg;
		}
		offset += end - arg + 1;
	}
	return NULL;
}

void ipc_reply(struct ipc_message *message, bool success, const char *response) {
	size_t response_size = strlen(response);
	if (response_size >= IPC_MESSAGE_MAX) {
		response_size = IPC_MESSAGE_MAX - 1;
	}
	char *packet = malloc(response_size + 1);
	if (packet) {
		packet[0] = success ? 0 : 1;
		memcpy(packet + 1, response, response_size);
		send(message->client_fd, packet, response_size + 1, MSG_NOSIGNAL);
		free(packet);
	}
	close(message->client_fd);
	free(message->data);
	message->client_fd = -1;
	message->data = NULL;
	message->size = 0;
}

int ipc_send_command(const char *socket_path, int argc, char **argv,
		char **response) {
	size_t packet_size = 0;
	for (int i = 0; i < argc; ++i) {
		packet_size += strlen(argv[i]) + 1;
	}
	if (packet_size == 0 || packet_size > IPC_MESSAGE_MAX) {
		fprintf(stderr, "IPC command is too long\n");
		return -1;
	}

	char *packet = malloc(packet_size);
	if (!packet) {
		return -1;
	}
	size_t offset = 0;
	for (int i = 0; i < argc; ++i) {
		size_t length = strlen(argv[i]) + 1;
		memcpy(packet + offset, argv[i], length);
		offset += length;
	}

	struct sockaddr_un addr;
	if (!set_socket_address(&addr, socket_path)) {
		free(packet);
		return -1;
	}
	int fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (fd == -1) {
		fprintf(stderr, "Unable to create IPC socket: %s\n", strerror(errno));
		free(packet);
		return -1;
	}
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		fprintf(stderr, "Unable to connect to %s: %s\n", socket_path,
			strerror(errno));
		close(fd);
		free(packet);
		return -1;
	}
	ssize_t sent = send(fd, packet, packet_size, MSG_NOSIGNAL);
	free(packet);
	if (sent != (ssize_t)packet_size) {
		fprintf(stderr, "Unable to send IPC command: %s\n",
			sent == -1 ? strerror(errno) : "short write");
		close(fd);
		return -1;
	}

	char response_packet[IPC_MESSAGE_MAX];
	ssize_t received = recv(fd, response_packet, sizeof(response_packet) - 1, 0);
	close(fd);
	if (received < 1) {
		fprintf(stderr, "Unable to receive IPC response: %s\n",
			received == 0 ? "empty message" : strerror(errno));
		return -1;
	}
	response_packet[received] = '\0';
	*response = strdup(response_packet + 1);
	return response_packet[0] == 0 ? 0 : 1;
}

void ipc_close_server(int server_fd, const char *path) {
	if (server_fd >= 0) {
		close(server_fd);
	}
	if (path) {
		unlink(path);
	}
}
