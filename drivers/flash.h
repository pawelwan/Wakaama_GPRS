#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f4xx.h"

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
#define DATA_ADDRESS             0x0807D000

typedef enum {
    BOOT_FLAG =     DATA_ADDRESS
} flag_type;

typedef enum {
    FLAG_ERROR =    0x00000000,
    FLAG_SET =      0x12345678,
    FLAG_RESET =    0xFFFFFFFF
} flag_state;

//clear sector containing 'addr', return 1 on succes
uint8_t flash_clear_sector(uint32_t addr);

//program 'size' elements of uint32_t 'data' at 'addr', return next empty address
uint32_t flash_program_by_word(uint32_t addr, uint32_t *data, uint32_t size);

//program 'size' elements of uint8_t 'data' at 'addr', return next empty address
uint32_t flash_program_by_byte(uint32_t addr, uint8_t *data, uint32_t size);

//set 'flag' to 'state'
FLASH_Status flash_set_flag(flag_type flag, flag_state state);

//get state of 'flag'
flag_state flash_get_flag(flag_type flag);

#endif /* __FLASH_H */
