#include <stdlib.h>
#include <stdio.h>
#include "axon.h"

main() {
	// open the file descriptor
	int fd = fpga_open();
	if (fd < 0) {
		printf ("can't open fpga: %d\n", fd);
		exit(-1);
	}

	uint32_t version = 0;
	int ret_val = fpga_get_version(fd, &version);
	if (ret_val < 0) {
		printf ("can't get version: %d\n", ret_val);
		exit(-1);
	}
	printf("fpga version: %zu\n", version);

	uint32_t status = 0;
	ret_val = fpga_get_status(fd, &status);
	if (ret_val < 0) {
		printf ("can't get status: %d\n", ret_val);
		exit(-1);
	}
	printf("fpga status: %zu\n", status);

	status = 0xFF;
	ret_val = fpga_set_status(fd, &status);
	if (ret_val < 0) {
		printf ("can't set status: %d\n", ret_val);
		exit(-1);
	}

	status = 0;
	ret_val = fpga_get_status(fd, &status);
	if (ret_val < 0) {
		printf ("can't get status (2): %d\n", ret_val);
		exit(-1);
	}
	printf("fpga status: %zu\n", status);
	fpga_close(fd);

	printf("wait until the fpga says something...\n");
	ret_val = fpga_wait_for_change();
	printf("fpga has spoken... %d\n", ret_val);
}
