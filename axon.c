#include "axon.h"
#include "module/io.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <poll.h>

int fpga_open() {
	return open(FPGA_DEVICE_FILE_NAME, 0);
}

void fpga_close(int file_desc) {
	close(file_desc);
}

int fpga_get_version(int file_desc, uint32_t* version) {
	int ret_val;

	ret_val = ioctl(file_desc, FPGA_IOCTL_GET_VERSION, version);
	if (ret_val < 0) {
		return ret_val;
	}
	return ret_val;
}

int fpga_get_status(int file_desc, uint32_t* status) {
	int ret_val;

	ret_val = ioctl(file_desc, FPGA_IOCTL_GET_STATUS, status);
	if (ret_val < 0) {
		return ret_val;
	}
	return ret_val;
}

int fpga_set_status(int file_desc, uint32_t* status) {
	int ret_val;

	ret_val = ioctl(file_desc, FPGA_IOCTL_SET_STATUS, status);
	if (ret_val < 0) {
		return ret_val;
	}
	return ret_val;
}

/**
 * fpga_wait_for_change blocks until the kernel module sends a signal
 *
 * @return -1 on error, 0 on timeout, 1 on signal
 */
int fpga_wait_for_change() {
	int notifyFd, retval;
	char value;
	struct pollfd ufds[1];
	// open the notify attribute file
	if ((notifyFd = open(FPGA_SYSFS_NOTIFY_PATH, O_RDWR)) < 0) {
		return -1;
	}

	// read, because opening an file is for poll a change
	read(notifyFd, &value, 1);

	ufds[0].fd = notifyFd;
	ufds[0].events = POLLIN;

	while(1) {
		ufds[0].revents = 0;

		retval = poll(ufds, 1, 1000);
		// Error
		if (retval < 0) {
			close(notifyFd);
			return -1;
		}
		// Timeout
		else if (retval == 0) {
			close(notifyFd);
			return 0;
		}
		// Signal from kernel module
		else if (ufds[0].revents & POLLERR) {
			close(notifyFd);
			return 1;
		}
	}
	
}
