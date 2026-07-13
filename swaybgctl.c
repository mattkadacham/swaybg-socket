#define _XOPEN_SOURCE 700

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipc.h"

static void usage(FILE *stream) {
	fprintf(stream,
		"Usage: swaybgctl <socket-path> <image-path>\n"
		"       swaybgctl <socket-path> set <image-path>\n"
		"       swaybgctl <socket-path> cache <id> <image-path>\n"
		"       swaybgctl <socket-path> show <id>\n"
		"       swaybgctl <socket-path> next\n"
		"       swaybgctl <socket-path> prev\n"
		"       swaybgctl <socket-path> drop <id>\n"
		"       swaybgctl <socket-path> clear\n"
		"       swaybgctl <socket-path> status\n");
}

int main(int argc, char **argv) {
	if (argc < 3) {
		usage(stderr);
		return EXIT_FAILURE;
	}

	char *resolved_path = NULL;
	char *direct_args[2] = { "set", NULL };
	char **command_args = argv + 2;
	int command_argc = argc - 2;
	bool known_command = strcmp(argv[2], "set") == 0 ||
		strcmp(argv[2], "cache") == 0 || strcmp(argv[2], "show") == 0 ||
		strcmp(argv[2], "next") == 0 || strcmp(argv[2], "prev") == 0 ||
		strcmp(argv[2], "drop") == 0 || strcmp(argv[2], "clear") == 0 ||
		strcmp(argv[2], "status") == 0;

	if (argc == 3 && !known_command) {
		resolved_path = realpath(argv[2], NULL);
		if (!resolved_path) {
			fprintf(stderr, "Unable to resolve image path %s: %s\n", argv[2],
				strerror(errno));
			return EXIT_FAILURE;
		}
		direct_args[1] = resolved_path;
		command_args = direct_args;
		command_argc = 2;
	} else if (strcmp(argv[2], "set") == 0) {
		if (argc != 4) {
			usage(stderr);
			return EXIT_FAILURE;
		}
		resolved_path = realpath(argv[3], NULL);
		if (!resolved_path) {
			fprintf(stderr, "Unable to resolve image path %s: %s\n", argv[3],
				strerror(errno));
			return EXIT_FAILURE;
		}
		argv[3] = resolved_path;
	} else if (strcmp(argv[2], "cache") == 0) {
		if (argc != 5) {
			usage(stderr);
			return EXIT_FAILURE;
		}
		resolved_path = realpath(argv[4], NULL);
		if (!resolved_path) {
			fprintf(stderr, "Unable to resolve image path %s: %s\n", argv[4],
				strerror(errno));
			return EXIT_FAILURE;
		}
		argv[4] = resolved_path;
	} else if (strcmp(argv[2], "show") == 0 ||
			strcmp(argv[2], "drop") == 0) {
		if (argc != 4) {
			usage(stderr);
			return EXIT_FAILURE;
		}
	} else if (strcmp(argv[2], "next") == 0 ||
			strcmp(argv[2], "prev") == 0 || strcmp(argv[2], "clear") == 0 ||
			strcmp(argv[2], "status") == 0) {
		if (argc != 3) {
			usage(stderr);
			return EXIT_FAILURE;
		}
	} else {
		usage(stderr);
		return EXIT_FAILURE;
	}

	char *response = NULL;
	int result = ipc_send_command(argv[1], command_argc, command_args, &response);
	free(resolved_path);
	if (response && response[0] != '\0') {
		fprintf(result == 0 ? stdout : stderr, "%s\n", response);
	}
	free(response);
	return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
