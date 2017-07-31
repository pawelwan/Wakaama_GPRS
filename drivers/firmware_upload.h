#ifndef __FIRMWARE_UPLOAD_H
#define __FIRMWARE_UPLOAD_H

#include "flash.h"
#include "stm32f4xx.h"

#define APPLICATION_SECTOR      FLASH_SECTOR_8_ADDR
#define LAST_SECTOR             FLASH_SECTOR_11_ADDR

typedef void (*pFunction)(void);

//erase flash sectors from APPLICATION_ADDRESS to LAST_SECTOR included
uint8_t initialize_firmware_upload();

//upload firmware batch of 'size' bytes
FLASH_Status upload_firmware_batch(void* data, uint32_t size);

//check if the uploaded firmware is correct
uint8_t check_firmware(uint32_t crc);

//run uploaded firmware
void run_firmware();

#endif /* __FIRMWARE_UPLOAD_H */
