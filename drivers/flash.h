#ifndef __FLASH_H
#define __FLASH_H

#define FLASH_SECTOR_0_ADDR      0x08000000          // Sector 0, 16 Kbytes
#define FLASH_SECTOR_1_ADDR      0x08004000          // Sector 1, 16 Kbytes
#define FLASH_SECTOR_2_ADDR      0x08008000          // Sector 2, 16 Kbytes
#define FLASH_SECTOR_3_ADDR      0x0800C000          // Sector 3, 16 Kbytes
#define FLASH_SECTOR_4_ADDR      0x08010000          // Sector 4, 64 Kbytes
#define FLASH_SECTOR_5_ADDR      0x08020000          // Sector 5, 128 Kbytes
#define FLASH_SECTOR_6_ADDR      0x08040000          // Sector 6, 128 Kbytes
#define FLASH_SECTOR_7_ADDR      0x08060000          // Sector 7, 128 Kbytes
#define FLASH_SECTOR_8_ADDR      0x08080000          // Sector 8, 128 Kbytes
#define FLASH_SECTOR_9_ADDR      0x080A0000          // Sector 9, 128 Kbytes
#define FLASH_SECTOR_10_ADDR     0x080C0000          // Sector 10, 128 Kbytes
#define FLASH_SECTOR_11_ADDR     0x080E0000          // Sector 11, 128 Kbytes

//erase all sectors between start_addr and start_addr + size included
FLASH_Status flash_erase(uint32_t start_addr, uint32_t size);

//program size bytes of data at start_addr
FLASH_Status flash_program(uint32_t start_addr, void* data, uint32_t size);

#endif /* __FLASH_H */
