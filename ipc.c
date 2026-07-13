#include <errno.h>
#include <limits.h>
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

char *ipc_receive_path(int server_fd) {
	int client_fd = accept(server_fd, NULL, NULL);
	if (client_fd == -1) {
		fprintf(stderr, "Unable to accept IPC connection: %s\n", strerror(errno));
		return NULL;
	}

	struct pollfd pfd = { .fd = client_fd, .events = POLLIN };
	if (poll(&pfd, 1, 1000) != 1) {
		fprintf(stderr, "IPC client did not send an image path\n");
		close(client_fd);
		return NULL;
	}

	char buffer[PATH_MAX];
	ssize_t size = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
	close(client_fd);
	if (size <= 0) {
		fprintf(stderr, "Unable to receive image path: %s\n",
			size == 0 ? "empty message" : strerror(errno));
		return NULL;
	}
	buffer[size] = '\0';
	return strdup(buffer);
}

int ipc_send_path(const char *socket_path, const char *image_path) {
	if (strlen(image_path) >= PATH_MAX) {
		fprintf(stderr, "Image path is too long\n");
		return -1;
	}

	struct sockaddr_un addr;
	if (!set_socket_address(&addr, socket_path)) {
		return -1;
	}

	int fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (fd == -1) {
		fprintf(stderr, "Unable to create IPC socket: %s\n", strerror(errno));
		return -1;
	}
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		fprintf(stderr, "Unable to connect to %s: %s\n", socket_path,
			strerror(errno));
		close(fd);
		return -1;
	}

	ssize_t length = strlen(image_path);
	ssize_t sent = send(fd, image_path, length, 0);
	close(fd);
	if (sent != length) {
		fprintf(stderr, "Unable to send image path: %s\n",
			sent == -1 ? strerror(errno) : "short write");
		return -1;
	}
	return 0;
}

void ipc_close_server(int server_fd, const char *path) {
	if (server_fd >= 0) {
		close(server_fd);
	}
	if (path) {
		unlink(path);
	}
}
