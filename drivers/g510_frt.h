/*
* The MIT License (MIT)
* Copyright (c) 2017 Robert Brzoza-Woch
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

#ifndef __G510_FRT_H__
#define __G510_FRT_H__

#define G510_USART		3


void g510Init(void);

int g510Start(void);

void g510TxCmd(const char* cmd);

int g510PowerOn(void);

void g510PowerOff(void);

void g510FlushRx(void);

int g510Creg(void);

int g510DownloadFileToBuf(const char* address, char* buf, uint32_t max_len);

int loadFile(const char* url, char* buf, uint32_t buf_len);

int getSMS(char* buf, uint32_t max_len);

int sendSMS(char* buf, uint32_t max_len);

int sendText(uint8_t socket, const char* s);

#define APN		"plus"
#define USER	"plusgsm"
#define PASSWD	"plusgsm"


#endif
