#include <linux/types.h>
#include "fpga_mapping.h"

#define FPGA_BASE_ADDRESS 0x90000000
#define FPGA_VERSION_ADDRESS FPGA_BASE_ADDRESS
#define FPGA_STATUS_ADDRESS FPGA_BASE_ADDRESS + 0x4

#define DEBUG

#ifdef DEBUG
uint32_t fpga_version_value = 0xA;
uint32_t fpga_status_value = 0xFF00;

uint32_t* fpga_version_ptr = &fpga_version_value;
uint32_t* fpga_status_ptr = &fpga_status_value;
#else
uint32_t* fpga_version_ptr = &FPGA_VERSION_ADDRESS;
uint32_t* fpga_status_ptr = &FPGA_STATUS_ADDRESS;
#endif

uint32_t fpga_get_version(void) {
	return *fpga_version_ptr;
}

uint32_t fpga_get_status(void) {
	return *fpga_status_ptr;
}

void fpga_set_status(uint32_t status) {
	*fpga_status_ptr = status;
}
