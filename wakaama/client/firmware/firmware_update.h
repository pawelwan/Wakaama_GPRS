#ifndef FIRMWARE_UPDATE_H
#define FIRMWARE_UPDATE_H

#include <stdio.h>
#include "wakaama/liblwm2m.h"

#define FIRMWARE_STRUCT_SIZE 256
#define FIRMWARE_STRUCT_OFFSET 0
#define FIRMWARE_PACKAGE_SIZE 1000

// when u edit that struct please change also in file gen_metadata.c
typedef struct OTAMetadata {
  uint32_t crc;             //
  uint32_t crc_shadow;      //
  uint32_t size;            //  Size of firmware image
  uint32_t uuid;             //  Integer representing unique firmware ID
  uint16_t version;         //  Integer representing firmware version
} OTAMetadata_t;

int downloadFirmware(const char * uri, lwm2m_context_t * ContextP);
int crc32(const char *message, long size);

#endif /* FIRMWARE_UPDATE_H */

