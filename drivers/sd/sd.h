#ifndef __SD_H__
#define __SD_H__

int sdInit(void);
int sdGetSizeInfo(uint32_t *NumberOfBlocks, uint16_t *BlockSize);
int sdReadBlock(uint32_t BlockAddress, uint8_t *Buffer);
int sdWriteBlock(uint32_t BlockAddress, uint8_t *Buffer);
int sdReadBlocks(uint32_t BlockAddress, uint8_t blocks, uint8_t *Buffer);
int sdWriteBlocks(uint32_t BlockAddress, uint8_t blocks, uint8_t *Buffer);


#endif

