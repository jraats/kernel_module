#ifndef AXON_H
#define AXON_H

#include <stdint.h>

int fpga_open();
void fpga_close(int file_desc);
int fpga_get_version(int file_desc, uint32_t* version);
int fpga_get_status(int file_desc, uint32_t* status);
int fpga_set_status(int file_desc, uint32_t* status);
int fpga_wait_for_change();

#endif // AXON_H
