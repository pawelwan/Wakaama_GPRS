/*Contiki is licensed under the 3-clause BSD license. This license gives
everyone the right to use and distribute the code, either in binary or
source code format, as long as the copyright license is retained in
the source code.

The copyright for different parts of the code is held by different
people and organizations, but the code is licensed under the same type
of license. The license text is:

 * Copyright (c) (Year), (Name of copyright holder)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
changed from crc16 to crc32
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include "crc32.c"

// when u edit that struct please change also in file firmware_update.h
typedef struct OTAMetadata {
  uint32_t crc;             //
  uint32_t crc_shadow;      //
  uint32_t size;            //  Size of firmware image
  uint32_t uuid;             //  Integer representing unique firmware ID
  uint16_t version;         //  Integer representing firmware version
} OTAMetadata_t;

FILE *firmware_bin; // firmware input .bin file
FILE *metadata_bin; // metadata output .bin file

const int i = 1;
#define is_bigendian() ( (*(char*)&i) == 0 )

uint32_t firmware_size = 0;

int
main(int argc, char *argv[]) {

  if ( !argv[1] ) {
    printf("Please provide a .bin file to compute the CRC on as the first argument.\n");
    return -1;
  }

  if ( !argv[2] ) {
    printf("Please provide a 16-bit hex firmware version integer as the second argument.\n");
    return -1;
  }

  if ( !argv[3] ) {
    printf("Please provide a 32-bit hex UUID integer as the third argument.\n");
    return -1;
  }

  if ( !argv[4] ) {
    printf("Please provide 0 or 1 to indicate whether this image is pre-verified or not as the fourth argument.\n");
    return -1;
  }

    //  (1) Open the firmware .bin file
  firmware_bin = fopen( argv[1], "rb" );
  int firmware_verified;
  sscanf( argv[4], "%d", &firmware_verified );

  fseek(firmware_bin, 0, SEEK_END);
  long fsize = ftell(firmware_bin);
  fseek(firmware_bin, 0, SEEK_SET);  //same as rewind(f);
  char *bin = ( char *)malloc(fsize + 1);
  fread(bin, fsize, 1, firmware_bin);
  fclose(firmware_bin);
  bin[fsize] = 0;

  uint32_t crc_result = crc32(bin, fsize);

  //  (2) Run the CRC32 calculation over the file.  Print result.
  printf( "size: %d sum: result %d\r\n", fsize, crc_result);

  //  (4) Generate OTA image metadata
  OTAMetadata_t metadata;
  metadata.crc = crc_result;
  if (firmware_verified) {
    metadata.crc_shadow = crc_result;
  } else {
    metadata.crc_shadow = 0;
  }
  metadata.size = firmware_size;
  sscanf( argv[2], "%xu", &(metadata.version) );
  sscanf( argv[3], "%xu", &(metadata.uuid) );
  uint8_t output_buffer[ sizeof(OTAMetadata_t) ];
  memcpy( output_buffer, (uint8_t *)&metadata, sizeof(OTAMetadata_t) );

  //swap_endian_32( &metadata.uuid );

  //  (5) Open the output firmware .bin file
  metadata_bin = fopen( "obj/firmware_metadata.bin", "wb" );

  //  (6) Write the metadata
  fwrite(output_buffer, sizeof(output_buffer), 1, metadata_bin);
  //TODO  it is possible to get rid of offset for our implmentation
  //  (7) 0xff spacing until firmware binary starts
  uint8_t blank_buffer[236];
  for (int b=0; b<236; b++) {
    blank_buffer[ b ] = 0xff;
  }
  fwrite( blank_buffer, 236, 1, metadata_bin);

  //  (8) Close the metadata file
  fclose( metadata_bin );

  return 0;
}
