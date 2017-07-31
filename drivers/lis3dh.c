#include "stm32f4xx.h"
#include "lis3dh.h"

#define TIMEOUT                 0x1000

#define LIS3DH_ADDR             0x30

//only read
#define LIS3DH_WHO_AM_I         0x0F
#define LIS3DH_OUT_X_L          0x28
#define LIS3DH_OUT_X_H          0x29
#define LIS3DH_OUT_Y_L          0x2A
#define LIS3DH_OUT_Y_H          0x2B
#define LIS3DH_OUT_Z_L          0x2C
#define LIS3DH_OUT_Z_H          0x2D

//read/write
#define LIS3DH_CTRL_REG0        0x1E
#define LIS3DH_CTRL_REG1        0x20
#define LIS3DH_CTRL_REG2        0x21
#define LIS3DH_CTRL_REG3        0x22
#define LIS3DH_CTRL_REG4        0x23
#define LIS3DH_CTRL_REG5        0x24
#define LIS3DH_CTRL_REG6        0x25

static uint8_t lis3dh_set_register(uint8_t);
static uint8_t lis3dh_read_register(uint8_t, void*, int);
static uint8_t lis3dh_write_register(uint8_t, uint8_t);
static float lis3dh_get_divider();

uint8_t lis3dh_init() {
    //clock enable for I2C1 peripheral
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    //reset signal to I2C1 peripheral
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
    //release reset signal to I2C1 peripheral
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);

    //connect PB8 to I2C1_SCL
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_I2C1);
    //connect PB9 to I2C1_SDA
    GPIO_PinAFConfig(GPIOB,  GPIO_PinSource9, GPIO_AF_I2C1);

    //GPIOB8 and GPIOB9 configuration
    GPIO_InitTypeDef gpio;
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9; // SCL, SDA
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_OD;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

    //apply GPIOB configuration
    GPIO_Init(GPIOB, &gpio);

    //I2C1 congifuration
    I2C_InitTypeDef i2c;
    I2C_StructInit(&i2c);
    i2c.I2C_ClockSpeed = 100000;
    i2c.I2C_Mode = I2C_Mode_I2C;
    i2c.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c.I2C_Ack = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    //apply I2C1 configuration
    I2C_Init(I2C1, &i2c);
    //enable I2C1 peripheral
    I2C_Cmd(I2C1, ENABLE);

    //check if accelerometer is responsive
    uint8_t res;
    if (!lis3dh_read_register(LIS3DH_WHO_AM_I, &res, 1)) return 0;
    if (res != 0x33) return 0;

    // 400Hz data rate (0111), normal mode (0), enable all axes (111)
    if (!lis3dh_write_register(LIS3DH_CTRL_REG1, 0x77)) return 0;
    // Block Until Update (10), scale selection - 2g (00), High resolution (1000)
    if (!lis3dh_write_register(LIS3DH_CTRL_REG4, 0x88)) return 0;

    return 1;
}

float lis3dh_get_axis(lis3dh_axis axis) {
    uint8_t tmp[2];
    if (!lis3dh_read_register((uint8_t) axis, tmp, 2)) return 32.0;

    int16_t value = tmp[0];
    value |= ((uint16_t) tmp[1]) << 8;

    float divider = lis3dh_get_divider();
    if (!((int) divider)) return 32.0;

    return value / divider;
}

uint8_t lis3dh_set_scale(lis3dh_scale scale) {
    uint8_t s;
    if (!lis3dh_read_register(LIS3DH_CTRL_REG4, &s, 1)) return 0;

    s &= ~(0x30);
    s |= scale << 4;

    if (!lis3dh_write_register(LIS3DH_CTRL_REG4, s)) return 0;

    return 1;
}

static uint8_t lis3dh_set_register(uint8_t reg) {
    uint32_t timeout = 4 * TIMEOUT;

    //check if I2C is not busy
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) == SET) {
        if (!(timeout--)) return 0;
    }

    //ST - start
    I2C_GenerateSTART(I2C1, ENABLE);
    while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS) {
        if (!(timeout--)) return 0;
    }

    //SAD+W - write to LIS3DH_ADDR device
    I2C_Send7bitAddress(I2C1, LIS3DH_ADDR, I2C_Direction_Transmitter);
    while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS) {
        if (!(timeout--)) return 0;
    }

    //SUB - set register to reg
    I2C_SendData(I2C1, reg);
    while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS) {
        if (!(timeout--)) return 0;
    }

    return 1;
}

static uint8_t lis3dh_read_register(uint8_t reg, void* data, int size) {
    int i;
    uint8_t* buffer = (uint8_t*) data;
    uint32_t timeout = (size + 3) * TIMEOUT;

    if (!lis3dh_set_register(0x80 | reg)) return 0;

    //SR - repeated start
    I2C_GenerateSTART(I2C1, ENABLE);
    while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS) {
        if (!(timeout--)) return 0;
    }

    //SAD+R - read from LIS3DH_ADDR device
    I2C_Send7bitAddress(I2C1, LIS3DH_ADDR, I2C_Direction_Receiver);
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR) == RESET) {
        if (!(timeout--)) return 0;
    }

    for (i = 0; i < size; i++) {
        if (i == size - 1) {
            //disable automatic ack, clear ADDR register and send stop
            I2C_AcknowledgeConfig(I2C1, DISABLE);
            (void) I2C1->SR2;
            I2C_GenerateSTOP(I2C1, ENABLE);

            //get data
            while (I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET) {
                if (!(timeout--)) return 0;
            }
            buffer[i] = I2C_ReceiveData(I2C1);
            continue;
        }

        //get data
        while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS) {
            if (!(timeout--)) return 0;
        }
        buffer[i] = I2C_ReceiveData(I2C1);

    }

    //wait for STOP bit and enable automatic ack
    while (I2C1->CR1 & I2C_CR1_STOP) {
        if (!(timeout--)) return 0;
    }
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return 1;
}

static uint8_t lis3dh_write_register(uint8_t reg, uint8_t value) {
    uint32_t timeout = 2 * TIMEOUT;

    if (!lis3dh_set_register(reg)) return 0;

    //send data
    I2C_SendData(I2C1, value);
    while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS) {
        if (!(timeout--)) return 0;
    }

    //clear ADDR register, send stop and wait for STOP bit
    (void) I2C1->SR2;
    I2C_GenerateSTOP(I2C1, ENABLE);
    while (I2C1->CR1 & I2C_CR1_STOP) {
        if (!(timeout--)) return 0;
    }

    return 1;
}

static float lis3dh_get_divider() {
    uint8_t res;

    if (!lis3dh_read_register(LIS3DH_CTRL_REG4, &res, 1)) return 0.0;

    lis3dh_scale scale = (lis3dh_scale) ((res >> 4) & 0b11);

    if (scale == LIS3DH_SCALE_16G) return 1365.0;       // different sensitivity at 16g
    else if (scale == LIS3DH_SCALE_8G) return 4096.0;
    else if (scale == LIS3DH_SCALE_4G) return 8192.0;
    else if (scale == LIS3DH_SCALE_2G) return 16384.0;

    return 0.0;
}
