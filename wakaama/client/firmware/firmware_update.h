#ifndef FIRMWARE_UPDATE_H
#define FIRMWARE_UPDATE_H

#include <stdio.h>
#include "wakaama/liblwm2m.h"

#define HEADER_SIZE sizeof(FirmwareHeader_t)
#define HEADER_OFFSET 0

// when u edit that struct please change also in file gen_header.c
typedef struct FirmwareHeader {
  uint8_t entry_sign;
  uint32_t crc;             //
  uint32_t crc_shadow;      //
  uint32_t size;            //  Size of firmware image
  uint32_t uuid;             //  Integer representing unique firmware ID
  uint16_t version;         //  Integer representing firmware version
  uint8_t closing_sign;
} FirmwareHeader_t;

int downloadFirmware(const char * uri, lwm2m_context_t * ContextP);
int crc32(const char *message, long size);

#endif /* FIRMWARE_UPDATE_H */

