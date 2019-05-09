#ifndef IO_H
#define IO_H

#include <linux/ioctl.h>

/* The major device number. We can't rely on dynamic registration any more, because ioctls need to know it. */
#define FPGA_MAJOR_NUM 241

/* Get the FPGA version */
#define FPGA_IOCTL_GET_VERSION _IOR(FPGA_MAJOR_NUM, 1, uint32_t*)

/* Get the FPGA status */
#define FPGA_IOCTL_GET_STATUS _IOR(FPGA_MAJOR_NUM, 2, uint32_t*)

/* Set the FPGA status */
#define FPGA_IOCTL_SET_STATUS _IOR(FPGA_MAJOR_NUM, 3, uint32_t*)

/* The name of the kernel module */
#define FPGA_DEVICE_NAME "fpga"

/* The name of the device file */
#define FPGA_DEVICE_FILE_NAME "/dev/fpga"

#define FPGA_SYSFS_NOTIFY_PATH "/sys/fpga/notify"
#define FPGA_SYSFS_NAME "fpga"
#define FPGA_SYSFS_ATTRIBUTE "notify"

#endif // IO_H
