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
#include "usart.h"
//#include "usart2.h"
#include "usart3.h"
//#include "usart4.h"
//#include "usart6.h"


void usart_shdn(uint32_t usart, uint32_t shdn_on)
{
	switch(usart)
	{
		case 2:
			//usart2_shdn(shdn_on);
			return;
		case 3:
			usart3_shdn(shdn_on);
			return;
/*		case 4:
			usart4_shdn(shdn_on);
			return;
		case 6:
			usart6_shdn(shdn_on);
			return;*/
	}
}

void usartInit(uint32_t usart, uint32_t baud)
{
	switch(usart)
	{
		case 2:
			//usart2Init(baud);
			return;
		case 3:
			usart3Init(baud);
			return;
/*		case 4:
			usart4Init(baud);
			return;
		case 6:
			usart6Init(baud);
			return;*/
	}
}

void usart_sleep(uint32_t usart, uint32_t ena)
{
	switch(usart)
	{
		case 2:
			//usart2_sleep(ena);
			return;
		case 3:
			usart3_sleep(ena);
			return;
/*		case 4:
			usart4_sleep(ena);
			return;
		case 6:
			usart6_sleep(ena);
			return;*/
	}
}


void usart_flush(uint32_t usart)
{
	switch(usart)
	{
		case 2:
			//while(usart2_inkey());
			return;
		case 3:
			while(usart3_inkey());
			return;
/*		case 4:
			while(usart4_inkey());
			return;
		case 6:
			while(usart6_inkey());
			return;*/
	}
}

int usart_chr(uint32_t usart, char chr)
{
	switch(usart)
	{
		/*case 2:
			return usart2_chr(chr);*/
		case 3:
			return usart3_chr(chr);
/*		case 4:
			return usart4_chr(chr);
		case 6:
			return usart6_chr(chr);*/
		default:
			return 0;
	}
}


int usart_waitkey(uint32_t usart, uint8_t* key, portTickType timeout)
{
	switch(usart)
	{
		/*case 2:
			return usart2_waitkey(key,timeout);*/
		case 3:
			return usart3_waitkey(key,timeout);
/*		case 4:
			return usart4_waitkey(key,timeout);
		case 6:
			return usart6_waitkey(key,timeout);*/
		default:
			return 0;
	}
}


char usart_inkey(uint32_t usart)
{
	switch(usart)
	{
		/*case 2:
			return usart2_inkey();*/
		case 3:
			return usart3_inkey();
/*		case 4:
			return usart4_inkey();
		case 6:
			return usart6_inkey();*/
		default:
			return 0;
	}
}

int usart_txBuf(uint32_t usart, const void * buf, uint32_t len)
{
	const uint8_t * ptr = buf;
	int res = 0;
	while(len--)
	{
		res = usart_chr(usart,*ptr++);
		if(res == 0) return 0;
	}
	return 1;
}

int usart_txt(uint32_t usart, const char * txt)
{
	switch(usart)
	{
		/*case 2:
			return usart2_txt(txt);*/
		case 3:
			return usart3_txt(txt);
/*		case 4:
			return usart4_txt(txt);
		case 6:
			return usart6_txt(txt);*/
		default:
			return 0;
	}
}


