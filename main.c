#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include "axon.h"

// overbodig?
#include <stdint.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <poll.h>
//end
#define TEST_SYSFS_TRIGGER  "/sys/hello/trigger"
#define TEST_SYSFS_NOTIFY   "/sys/hello/notify"

main() {
int cnt, notifyFd, triggerFd, rv;
/*    char attrData[100];
    struct pollfd ufds[2];

    // Open a connection to the attribute file.
    if ((notifyFd = open(TEST_SYSFS_NOTIFY, O_RDWR)) < 0)
    {
        perror("Unable to open notify");
        exit(1);
    }
    // Open a connection to the attribute file.
    if ((triggerFd = open(TEST_SYSFS_TRIGGER, O_RDWR)) < 0)
    {
        perror("Unable to open trigger");
        exit(1);
    }

    cnt = read( notifyFd, attrData, 100 );
   cnt = read( triggerFd, attrData, 100 );
//	char dummybuf = 0;
//	read(notifyFd, &dummybuf, 1);
//	read(triggerFd, &dummybuf, 1);

    ufds[0].fd = notifyFd;
    ufds[0].events = POLLIN;
    ufds[1].fd = triggerFd;
    ufds[1].events = POLLIN;

    // Someone suggested dummy reads before the poll() call
    ufds[0].revents = 0;
    ufds[1].revents = 0;

while(1) {
	printf("start poll..\n");
    if (( rv = poll( ufds, 2, 10000)) < 0 )
    {
        perror("poll error");
	break;
    }
    else if (rv == 0)
    {
        printf("Timeout occurred!\n");
		break;
    }
    else if (ufds[0].revents & POLLERR)
    {
        printf("triggered\n");
        cnt = read( notifyFd, attrData, 1 );
        printf( "Attribute file value: %02X (%c) [%d]\n", attrData[0], attrData[0], cnt );
	break;
	}
    printf( "revents[0]: %08X\n", ufds[0].revents );
    printf( "revents[1]: %08X\n", ufds[1].revents );
}
printf("woken\n");
    close( triggerFd );
    close( notifyFd );*/

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
