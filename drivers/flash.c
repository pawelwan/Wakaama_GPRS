#include "common.h"
#include "flash.h"

static uint32_t flash_get_sector(uint32_t);

FLASH_Status flash_erase(uint32_t start_addr, uint32_t size) {
    //unlock the FLASH control register access
    FLASH_Unlock();

    //Clear all FLASH pending flags
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    uint32_t start_sector = flash_get_sector(start_addr);
    uint32_t end_sector = flash_get_sector(start_addr + size);

    uint32_t current_sector;

    for (current_sector = start_sector; current_sector <= end_sector; current_sector += 8) {
        //VoltageRange_3 - operation will be done by word (32-bit)
        FLASH_Status res = FLASH_EraseSector(current_sector, VoltageRange_3);
        if (res != FLASH_COMPLETE) return res;
    }
    //lock the FLASH control register access
    FLASH_Lock();

    return FLASH_COMPLETE;
}

FLASH_Status flash_program(uint32_t start_addr, void* data, uint32_t size) {
    FLASH_Status res = flash_erase(start_addr, size);
    if (res != FLASH_COMPLETE) return res;

    uint32_t* buffer = (uint32_t*) data;
    uint32_t buffer_size = (size / 4);

    FLASH_Unlock();

    uint32_t i;
    uint32_t current_addr;
    for (i = 0; i < buffer_size; ++i) {
        current_addr = start_addr + 4 * i;
        res = FLASH_ProgramWord(current_addr, buffer[i]);
        if (res != FLASH_COMPLETE) return res;
    }

    uint8_t* buf = (uint8_t*) data;
    for(i = 0; i < (size % 4); ++i){
        current_addr = start_addr + 4 * buffer_size + i;
        res = FLASH_ProgramByte(current_addr, buf[4 * buffer_size + i]);
        if (res != FLASH_COMPLETE) return res;
    }

    FLASH_Lock();

    return FLASH_COMPLETE;
}

static uint32_t flash_get_sector(uint32_t addr) {
    uint32_t sector = 0;

    if((addr >= FLASH_SECTOR_0_ADDR) && (addr < FLASH_SECTOR_1_ADDR))
        sector = FLASH_Sector_0;
    else if((addr >= FLASH_SECTOR_1_ADDR) && (addr < FLASH_SECTOR_2_ADDR))
        sector = FLASH_Sector_1;
    else if((addr >= FLASH_SECTOR_2_ADDR) && (addr < FLASH_SECTOR_3_ADDR))
        sector = FLASH_Sector_2;
    else if((addr >= FLASH_SECTOR_3_ADDR) && (addr < FLASH_SECTOR_4_ADDR))
        sector = FLASH_Sector_3;
    else if((addr >= FLASH_SECTOR_4_ADDR) && (addr < FLASH_SECTOR_5_ADDR))
        sector = FLASH_Sector_4;
    else if((addr >= FLASH_SECTOR_5_ADDR) && (addr < FLASH_SECTOR_6_ADDR))
        sector = FLASH_Sector_5;
    else if((addr >= FLASH_SECTOR_6_ADDR) && (addr < FLASH_SECTOR_7_ADDR))
        sector = FLASH_Sector_6;
    else if((addr >= FLASH_SECTOR_7_ADDR) && (addr < FLASH_SECTOR_8_ADDR))
        sector = FLASH_Sector_7;
    else if((addr >= FLASH_SECTOR_8_ADDR) && (addr < FLASH_SECTOR_9_ADDR))
        sector = FLASH_Sector_8;
    else if((addr >= FLASH_SECTOR_9_ADDR) && (addr < FLASH_SECTOR_10_ADDR))
        sector = FLASH_Sector_9;
    else if((addr >= FLASH_SECTOR_10_ADDR) && (addr < FLASH_SECTOR_11_ADDR))
        sector = FLASH_Sector_10;
    else
        sector = FLASH_Sector_11;

    return sector;
}
