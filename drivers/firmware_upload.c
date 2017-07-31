#include "common.h"
#include "firmware_upload.h"

uint32_t current_addr = 0;

static uint32_t crc32(uint8_t *, uint32_t);

uint8_t initialize_firmware_upload() {
    for (uint32_t current_sector = APPLICATION_SECTOR; current_sector <= LAST_SECTOR; current_sector += 0x20000) {
        FLASH_Status res = flash_clear_sector(current_sector);
        if (res != FLASH_COMPLETE) return 0;
    }

    current_addr = APPLICATION_SECTOR;

    return 1;
}

uint8_t upload_firmware_batch(void *data, uint32_t size) {
    if (current_addr == 0) return 0;

    uint32_t res;
    uint32_t buffer_size = size / 4;

    res = flash_program_by_word(current_addr, (uint32_t *) data, buffer_size);
    if (!res) {
        current_addr = 0;
        return 0;
    }

    current_addr = res;
    uint8_t *buffer = (uint8_t *) (((uint32_t *) data) + buffer_size);
    buffer_size = size % 4;
    if (buffer_size == 0) {
        return 1;
    }

    res = flash_program_by_byte(current_addr, buffer, buffer_size);
    if (!res) {
        current_addr = 0;
        return 0;
    }

    current_addr = res;

    return 1;
}

uint8_t check_firmware(uint32_t crc) {
    if (current_addr == 0) return 0;

    uint32_t calculated_crc = crc32((uint8_t *) APPLICATION_SECTOR, current_addr - APPLICATION_SECTOR);

    if (calculated_crc == crc) {
        flash_set_flag(BOOT_FLAG, FLAG_SET);
    }
    else {
        flash_set_flag(BOOT_FLAG, FLAG_RESET);
    }

    return calculated_crc == crc;
}

void run_firmware() {
    uint32_t appStack = (uint32_t) *((uint32_t *) APPLICATION_SECTOR);

    pFunction appEntry = (pFunction) *(uint32_t *) (APPLICATION_SECTOR + 4);

    SCB->VTOR = APPLICATION_SECTOR;

    __set_MSP(appStack);

    appEntry();
}

static uint32_t crc32(uint8_t *message, uint32_t size) {
    uint32_t byte, crc, mask;

    crc = 0xFFFFFFFF;
    for(uint32_t i = 0; i < size; ++i) {
        byte = message[i];
        crc = crc ^ byte;
        for (int8_t j = 7; j >= 0; --j) {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}
