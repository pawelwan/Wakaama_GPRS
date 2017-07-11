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

#ifndef __USART_H__
#define __USART_H__

void usartInit(uint32_t usart, uint32_t baud);
void usart_sleep(uint32_t usart, uint32_t ena);
int usart_chr(uint32_t usart, char chr);
int usart_waitkey(uint32_t usart, uint8_t* key, portTickType timeout);
char usart_inkey(uint32_t usart);
int usart_txt(uint32_t usart, const char * txt);
void usart_flush(uint32_t usart);
void usart_shdn(uint32_t usart, uint32_t shdn_on);
int usart_txBuf(uint32_t usart, const void * buf, uint32_t len);

#endif
