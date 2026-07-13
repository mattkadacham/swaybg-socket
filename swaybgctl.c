#define _XOPEN_SOURCE 700

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipc.h"

static void usage(FILE *stream) {
	fprintf(stream,
		"Usage: swaybgctl [--socket <path>] <image-path>\n"
		"       swaybgctl [--socket <path>] set <image-path>\n"
		"       swaybgctl [--socket <path>] cache <id> <image-path>\n"
		"       swaybgctl [--socket <path>] show <id>\n"
		"       swaybgctl [--socket <path>] next\n"
		"       swaybgctl [--socket <path>] prev\n"
		"       swaybgctl [--socket <path>] drop <id>\n"
		"       swaybgctl [--socket <path>] clear\n"
		"       swaybgctl [--socket <path>] status\n"
		"\n"
		"The default socket is $XDG_RUNTIME_DIR/swaybg.sock.\n");
}

static bool is_command(const char *value) {
	return strcmp(value, "set") == 0 || strcmp(value, "cache") == 0 ||
		strcmp(value, "show") == 0 || strcmp(value, "next") == 0 ||
		strcmp(value, "prev") == 0 || strcmp(value, "drop") == 0 ||
		strcmp(value, "clear") == 0 || strcmp(value, "status") == 0;
}

int main(int argc, char **argv) {
	static struct option long_options[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "socket", required_argument, NULL, 's' },
		{ 0, 0, 0, 0 },
	};

	const char *socket_path = NULL;
	int option;
	while ((option = getopt_long(argc, argv, "hs:", long_options, NULL)) != -1) {
		switch (option) {
		case 's':
			socket_path = optarg;
			break;
		case 'h':
			usage(stdout);
			return EXIT_SUCCESS;
		default:
			usage(stderr);
			return EXIT_FAILURE;
		}
	}

	if (optind >= argc) {
		usage(stderr);
		return EXIT_FAILURE;
	}
	int command_index = optind;
	if (!socket_path && argc - optind >= 2 && !is_command(argv[optind])) {
		// Preserve the original positional socket syntax.
		socket_path = argv[optind];
		++command_index;
	}

	char default_socket[PATH_MAX];
	if (!socket_path) {
		const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
		if (!runtime_dir || runtime_dir[0] == '\0') {
			fprintf(stderr, "XDG_RUNTIME_DIR is not set; use --socket <path>\n");
			return EXIT_FAILURE;
		}
		int length = snprintf(default_socket, sizeof(default_socket),
			"%s/swaybg.sock", runtime_dir);
		if (length < 0 || (size_t)length >= sizeof(default_socket)) {
			fprintf(stderr, "Default socket path is too long\n");
			return EXIT_FAILURE;
		}
		socket_path = default_socket;
	}

	int command_argc = argc - command_index;
	char **command_args = argv + command_index;
	const char *command = command_args[0];
	bool known_command = is_command(command);
	char *resolved_path = NULL;
	char *direct_args[2] = { "set", NULL };
	if (command_argc == 1 && !known_command) {
		resolved_path = realpath(command, NULL);
		if (!resolved_path) {
			fprintf(stderr, "Unable to resolve image path %s: %s\n", command,
				strerror(errno));
			return EXIT_FAILURE;
		}
		direct_args[1] = resolved_path;
		command_args = direct_args;
		command_argc = 2;
	} else if (strcmp(command, "set") == 0) {
		if (command_argc != 2) {
			usage(stderr);
			return EXIT_FAILURE;
		}
		resolved_path = realpath(command_args[1], NULL);
		if (!resolved_path) {
			fprintf(stderr, "Unable to resolve image path %s: %s\n",
				command_args[1], strerror(errno));
			return EXIT_FAILURE;
		}
		command_args[1] = resolved_path;
	} else if (strcmp(command, "cache") == 0) {
		if (command_argc != 3) {
			usage(stderr);
			return EXIT_FAILURE;
		}
		resolved_path = realpath(command_args[2], NULL);
		if (!resolved_path) {
			fprintf(stderr, "Unable to resolve image path %s: %s\n",
				command_args[2], strerror(errno));
			return EXIT_FAILURE;
		}
		command_args[2] = resolved_path;
	} else if (strcmp(command, "show") == 0 ||
			strcmp(command, "drop") == 0) {
		if (command_argc != 2) {
			usage(stderr);
			return EXIT_FAILURE;
		}
	} else if (strcmp(command, "next") == 0 ||
			strcmp(command, "prev") == 0 || strcmp(command, "clear") == 0 ||
			strcmp(command, "status") == 0) {
		if (command_argc != 1) {
			usage(stderr);
			return EXIT_FAILURE;
		}
	} else {
		usage(stderr);
		return EXIT_FAILURE;
	}

	char *response = NULL;
	int result = ipc_send_command(socket_path, command_argc, command_args,
		&response);
	free(resolved_path);
	if (response && response[0] != '\0') {
		fprintf(result == 0 ? stdout : stderr, "%s\n", response);
	}
	free(response);
	return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
