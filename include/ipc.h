#ifndef _SWAYBG_IPC_H
#define _SWAYBG_IPC_H

int ipc_open_server(const char *path);
char *ipc_receive_path(int server_fd);
int ipc_send_path(const char *socket_path, const char *image_path);
void ipc_close_server(int server_fd, const char *path);

#endif
