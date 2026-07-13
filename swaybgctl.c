#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "ipc.h"

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: swaybgctl <socket-path> <image-path>\n");
		return EXIT_FAILURE;
	}
	char *image_path = realpath(argv[2], NULL);
	if (!image_path) {
		fprintf(stderr, "Unable to resolve image path %s: %s\n", argv[2],
			strerror(errno));
		return EXIT_FAILURE;
	}
	int result = ipc_send_path(argv[1], image_path);
	free(image_path);
	return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
