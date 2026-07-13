#ifndef _SWAYBG_IPC_H
#define _SWAYBG_IPC_H

#include <stdbool.h>
#include <stddef.h>

#define IPC_MESSAGE_MAX (64 * 1024)

struct ipc_message {
	int client_fd;
	char *data;
	size_t size;
};

int ipc_open_server(const char *path);
int ipc_receive_message(int server_fd, struct ipc_message *message);
const char *ipc_message_arg(const struct ipc_message *message, size_t index);
void ipc_reply(struct ipc_message *message, bool success, const char *response);
int ipc_send_command(const char *socket_path, int argc, char **argv,
	char **response);
void ipc_close_server(int server_fd, const char *path);

#endif
