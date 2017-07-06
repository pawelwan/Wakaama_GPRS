/*
* The MIT License (MIT)
* Copyright (c) 2016 Robert Brzoza-Woch
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "common.h"
#include "board.h"


static volatile uint8_t gprsState = 0;



uint16_t brdGetMainSupplyVoltage(void)
{
	//uint16_t adc = adcGet(6);
//	return ((uint16_t)((float)adc * 0.4536));
//	return ((uint16_t)((float)adc / 2.23));
return 0;
}








void brdInit(void)
{
	//outputs, GPIOA
    GPIO_Init(GPIOA, &(GPIO_InitTypeDef) {
        .GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_7,
        .GPIO_Mode  = GPIO_Mode_OUT,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        .GPIO_Speed = GPIO_Speed_50MHz,
    });

	//inputs, GPIOA
    GPIO_Init(GPIOA, &(GPIO_InitTypeDef) {
        .GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_4 | GPIO_Pin_6,
        .GPIO_Mode  = GPIO_Mode_IN,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL,
    });

	//outputs, GPIOB
    GPIO_Init(GPIOB, &(GPIO_InitTypeDef) {
        .GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_11,
        .GPIO_Mode  = GPIO_Mode_OUT,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        .GPIO_Speed = GPIO_Speed_50MHz,
    });

	//inputs, GPIOB
    GPIO_Init(GPIOB, &(GPIO_InitTypeDef) {
        .GPIO_Pin   = GPIO_Pin_5,
        .GPIO_Mode  = GPIO_Mode_IN,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL,
    });

	//outputs, GPIOD
    GPIO_Init(GPIOD, &(GPIO_InitTypeDef) {
        .GPIO_Pin   = GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13 | GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_4 | GPIO_Pin_1 | GPIO_Pin_0,
        .GPIO_Mode  = GPIO_Mode_OUT,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        .GPIO_Speed = GPIO_Speed_50MHz,
    });

    //dtr, ~pwrkey, ~res
    GPIO_SetBits(GPIOD,GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_1);

    //inputs, GPIOD
    GPIO_Init(GPIOD, &(GPIO_InitTypeDef) {
        .GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_3,
        .GPIO_Mode  = GPIO_Mode_IN,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL,
    });

	//outputs, GPIOE
    GPIO_Init(GPIOE, &(GPIO_InitTypeDef) {
        .GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_8,
        .GPIO_Mode  = GPIO_Mode_OUT,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        .GPIO_Speed = GPIO_Speed_50MHz,
    });


}


uint8_t brdReadOdInput(uint8_t in)
{
	uint8_t res = 0;
    //enable pullup
    GPIO_SetBits(GPIOA,GPIO_Pin_5);

    vTaskDelay(10);

	switch(in)
	{
		case 0:
			if( GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4) == Bit_SET ) res = 1;
			break;
		case 1:
			if( GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6) == Bit_SET ) res = 1;
			break;
	}

    GPIO_ResetBits(GPIOA,GPIO_Pin_5);

	return res;
}


void brdLcdSupply(int on)
{
	if(on)
	{
		GPIO_SetBits(GPIOE,GPIO_Pin_10);
	}
	else
	{
		GPIO_ResetBits(GPIOE,GPIO_Pin_10);
	}
}

void brdRfidSupply(int on)
{
	if(on)
	{
		GPIO_SetBits(GPIOE,GPIO_Pin_8);
	}
	else
	{
		GPIO_ResetBits(GPIOE,GPIO_Pin_8);
	}
}

void brdDistanceSensorSupply(int on)
{
	if(on)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_11);
	}
	else
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_11);
	}
}


void brdAuxSupply(int on)
{
	if(on)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_9);
	}
	else
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_9);
	}
}



void brdGprsSupply(int on)
{
	if(on)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_8);
	}
	else
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_8);
	}
}


int brdGprsRing(void)
{
	if( GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_3) == Bit_SET )
		return 1;
	else
		return 0;
}

void brdGprsPwrkey(int h)
{
	//xprintf("pwrkey set to %d\n",h);
	if(h)
		GPIO_SetBits(GPIOD,GPIO_Pin_7);
	else
		GPIO_ResetBits(GPIOD,GPIO_Pin_7);
}


void brdGprsRes(int h)
{
	if(h)
		GPIO_SetBits(GPIOD,GPIO_Pin_1);
	else
		GPIO_ResetBits(GPIOD,GPIO_Pin_1);
}


void brdGprsDtr(int h)
{
	if(h)
		GPIO_SetBits(GPIOD,GPIO_Pin_6);
	else
		GPIO_ResetBits(GPIOD,GPIO_Pin_6);
}


void led1(int action)
{
	switch(action)
	{
		case LED_ON:		GPIO_SetBits(GPIOD,GPIO_Pin_15); break;
		case LED_OFF:		GPIO_ResetBits(GPIOD,GPIO_Pin_15); break;
		case LED_TOGGLE:	GPIO_ToggleBits(GPIOD,GPIO_Pin_15); break;
	}
}

void led2(int action)
{
	switch(action)
	{
		case LED_ON:		GPIO_SetBits(GPIOD,GPIO_Pin_14); break;
		case LED_OFF:		GPIO_ResetBits(GPIOD,GPIO_Pin_14); break;
		case LED_TOGGLE:	GPIO_ToggleBits(GPIOD,GPIO_Pin_14); break;
	}
}

void led3(int action)
{
	switch(action)
	{
		case LED_ON:		GPIO_SetBits(GPIOD,GPIO_Pin_13); break;
		case LED_OFF:		GPIO_ResetBits(GPIOD,GPIO_Pin_13); break;
		case LED_TOGGLE:	GPIO_ToggleBits(GPIOD,GPIO_Pin_13); break;
	}
}
