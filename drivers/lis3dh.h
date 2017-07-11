#ifndef __LIS3DH_H
#define __LIS3DH_H

#include "stm32f4xx.h"

typedef enum {
    LIS3DH_SCALE_16G =  0b11,
    LIS3DH_SCALE_8G =   0b10,
    LIS3DH_SCALE_4G =   0b01,
    LIS3DH_SCALE_2G =   0b00    //default
} lis3dh_scale;

typedef enum {
    LIS3DH_X_AXIS =     0x28,
    LIS3DH_Y_AXIS =     0x2A,
    LIS3DH_Z_AXIS =     0x2C
} lis3dh_axis;

//initialization of LIS3DH accelerometer (return 1 if success)
uint8_t lis3dh_init();

//return x/y/z-axis accelaration data (return over 16.0 if error)
float lis3dh_get_axis(lis3dh_axis axis);

//set full scale from LIS3DH_SCALE_2G to LIS3DH_SCALE_16G (return 1 if success)
uint8_t lis3dh_set_scale(lis3dh_scale scale);

#endif /* __LIS3DH_H */
