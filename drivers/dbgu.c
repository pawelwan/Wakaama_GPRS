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
#include "stm32f4xx.h"
#include "dbgu.h"
#include "common.h"

#define USARTx USART1


//set to nonzero for full implementation (more code)
#define FULL						0
#define USE_RS485					0

//private tools


void dbg_shdn(uint32_t shdn_on)
{
	if(shdn_on)
	{
		RCC->APB2ENR &=~RCC_APB2Periph_USART1;
	}
	else
	{
		RCC->APB2ENR |= RCC_APB2Periph_USART1;
	}
	
	
}


void debug_init_default(void)
{
	// Enable peripheral clocks
	//
	RCC->AHB1ENR |= RCC_AHB1Periph_GPIOA;
	RCC->APB2ENR |= RCC_APB2Periph_USART1;
	
	// Initialize Serial Port
	//
	GPIO_Init(GPIOA, &(GPIO_InitTypeDef) {
		.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10,
		.GPIO_Mode = GPIO_Mode_AF,
		.GPIO_PuPd = GPIO_PuPd_UP
	});
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	
	USART_Init(USART1, &(USART_InitTypeDef) {
		.USART_BaudRate = 115200,
		.USART_WordLength = USART_WordLength_8b,
		.USART_StopBits = USART_StopBits_1,
		.USART_Parity = USART_Parity_No ,
		.USART_HardwareFlowControl = USART_HardwareFlowControl_None,
		.USART_Mode = USART_Mode_Rx | USART_Mode_Tx
	});
	
	USART_Cmd(USART1, ENABLE);
}

int debug_test(void)
{
	return ( USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET ) ? 0 : 1;
}


//send chr via UART (platform dependent)
void debug_chr(char chr)
{
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET) { ; }
	USART_SendData(USARTx, (uint16_t)chr);
}

int putChar(int ch)
{
	debug_chr(ch);
	return ch;
}



//returns ascii value of last char received
//returns 0 if no char was received since last debug_inkey call
//(platform dependent)
char debug_inkey(void)
{
	if(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET)
		return(0);
	else
		return (unsigned char)USART_ReceiveData(USARTx);
}

//halts program/task execution until char is received
//(platform dependent)
char debug_waitkey(void)
{
	while(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET) { ; }
	return (unsigned char)USART_ReceiveData(USARTx);
}




void debug_dump(void *address, uint16_t len)
{
	uint8_t *buf = address;
	const uint16_t bytesInLine = 16;
	const uint16_t spaceBetweenDumpAndASCII = 4;
	uint16_t i, counter=len;
	
	xprintf("Debug dump @ %08X\n",(unsigned int)address);
	
	while(1)
	{
		//insert last line (may be shorter than full line)
		if(counter < bytesInLine)
		{
			//debug_txt("\r\n"); debug32_t((uint32_t)buf); debug_txt(" "); debug16_t(len-counter); debug_txt(":   ");
			xprintf("%08X %04X: ",(unsigned int)buf,(unsigned int)(len-counter));
			
			//contents in hex
			for(i=0;i<bytesInLine;i++)
			{
				if(i<counter)
				{
					xprintf("%02X ",(unsigned int)(buf[i]));
				}
				else
				{
					xprintf("   ");
				}
				if(i%8==7) xprintf(" ");
			}
			
			//space
			for(i=0;i<spaceBetweenDumpAndASCII;i++)
			{
				xprintf(" ");
			}
			
			//contents in ASCII
			for(i=0;i<bytesInLine;i++)
			{
				if(i<counter)
				{
					debug_ascii(buf[i]);
				}
				else
				{
					debug_chr(' ');
				}
			}
			
			debug_chr('\n');
			
			break;
		}
		
		//debug_txt("\r\n"); debug32_t((uint32_t)buf); debug_txt(" "); debug16_t(len-counter); debug_txt(":   ");
		xprintf("%08X %04X:   ",(unsigned int)buf,(unsigned int)(len-counter));
		
		
		for(i=0;i<bytesInLine;i++)
		{
			xprintf("%02X ",(unsigned int)(buf[i]));
			if(i%8==7) debug_chr(' ');
		}
		
		//space
		for(i=0;i<spaceBetweenDumpAndASCII;i++)
		{
			debug_chr(' ');
		}
		
		//contents in ASCII
		for(i=0;i<bytesInLine;i++)
		{
			debug_ascii(buf[i]);
		}
		
		buf += bytesInLine;
		if(counter >= bytesInLine)
		{
			counter -= bytesInLine;
		}
		
		debug_chr('\n');
			
		if(counter == 0) break;
		
	}	//while(counter)
	//footer
	xprintf("End of dump");
	
}

