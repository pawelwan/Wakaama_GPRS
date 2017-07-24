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

#include "common.h"
#include <stdlib.h>
#include "board.h"
#include "usart.h"
#include "g510_frt.h"



int g510WaitForResponse(const char* response, uint32_t timeout)
{
	uint8_t key;
	int idx = 0;
	uint8_t act = 0;

	//xprintf("g510WFR: %s ..., actual: ",response);

	while(usart_waitkey(G510_USART,&key,timeout))
	{
		//xprintf("%c",key);
		if(act==0)
		{
			if(key==response[0])
			{
				act = 1;
				idx++;
			}
		}
		else
		{
			//xprintf("act, idx=%d, resp[idx]=%c\n",idx,response[idx]);
			if( response[idx] == 0 )
			{
				//xprintf("end of resp reached\n");
				//xprintf("\nOK\n");
				return 0;
			}

			if(response[idx] != key)
			{
				//xprintf("failed at idx=%d, key=%c, res[idx]=%c\n",idx,key,response[idx]);
				//xprintf("\nfailed\n");
				return -100-idx;
			}

			idx++;
		}
	}

	//xprintf("wait exit\n");

	return -1000;
}





int g510PowerOn(void)
{
	brdGprsSupply(1);
	brdGprsPwrkey(1);
	brdGprsDtr(1);
	brdGprsRes(1);
	vTaskDelay(1000);
	brdGprsPwrkey(0);
	vTaskDelay(1000);
	brdGprsPwrkey(1);


	#if 0
	for(int i=0;i<10;i++)
	{
		g510TxCmd("AT\r");
		if(g510WaitForResponse("OK",1000)==0) break;
	}
	#else
	if(g510WaitForResponse("AT command ready",60000)) return -1;
	#endif


	g510TxCmd("AT\r");
	if(g510WaitForResponse("OK",1000)) return -2;

	g510TxCmd("ATE000\r");
	if(g510WaitForResponse("OK",1000)) return -3;

	g510TxCmd("AT\r");
	if(g510WaitForResponse("OK",1000)) return -4;

	//xprintf("waiting for sim ready code...\n");

	if(g510WaitForResponse("+SIM READY",60000)) return -2;


	return 0;
}


void g510PowerOff(void)
{
	brdGprsPwrkey(0);
	vTaskDelay(1000);
	brdGprsPwrkey(1);
	vTaskDelay(1000);
	brdGprsDtr(0);
	brdGprsRes(0);
	brdGprsSupply(0);
}


void g510TxCmd(const char* cmd)
{
	//xprintf("issuing cmd: %s\n",cmd);
	int len = strlen(cmd);
	usart_txBuf(G510_USART,(const uint8_t*)cmd,len);
}


void g510Init(void)
{
	usartInit(3,115200);
	//g510IoInit();
}


int g510ReadUntilChar(uint8_t c)
{
	uint8_t chr;

	do
	{
		if(usart_waitkey(G510_USART,&chr,1)==0) return -1;
		//xprintf("flushing %c\n",chr);
	}while(chr != c);

	return 0;
}


int readToBufUntilChar(uint8_t c, uint8_t* buf, uint16_t max_len)
{
	uint8_t chr;

	do
	{
		if(usart_waitkey(G510_USART,&chr,1)==0) return -1;
		if(max_len)
		{
			max_len--;
			*buf++ = chr;
		}
		else
		{
			return 1;
		}
	}while(chr != c);

	return 0;
}


int g510ReadResp(char* buf, uint16_t max_len, uint32_t timeout)
{
	uint8_t chr;
	uint16_t len = 0;
	int res = 0;

	res = usart_waitkey(G510_USART,&chr,timeout);
	if(res)
	{
		buf[len++] = chr;
		if(len>=max_len)
		{
			len--;
			buf[len] = 0;
			return len;
		}
	}

	do
	{
		res = usart_waitkey(G510_USART,&chr,250);
		if(res)
		{
			buf[len++] = chr;

			if(len>=max_len)
			{
				len--;
				break;
			}

		}
	}while(res);

	buf[len] = 0;

	return len;
}


static void txByteHex(uint8_t b)
{
	char buf[3];
	sprintf(buf,"%02X",b);
	usart_txBuf(G510_USART,(uint8_t*)buf,2);
}


static void txString(const char* s) __attribute__((unused));
static void txString(const char* s)
{
	while(*s) txByteHex(*s++);
}

void txBinData(const uint8_t* data, uint16_t len)
{
	for(uint16_t i=0;i<len;i++)
	{
		txByteHex(data[i]);
		//xprintf("txing %c\n",data[i]);
	}
	/*while(len--)
	{
		txBinByte(*data++);
	}*/
}

int g510RxBinByte(uint8_t *b)
{
	int res = 0;
	char bfr[3];
	res += usart_waitkey(G510_USART,(uint8_t*)&bfr[0],100);
	res += usart_waitkey(G510_USART,(uint8_t*)&bfr[1],100);
	bfr[2]=0;

	if(bfr[0]== '\r') return 1;
	if(bfr[0]== '\n') return 2;
	if(bfr[1]== '\r') return 3;
	if(bfr[1]== '\n') return 4;

	char* end;
	uint8_t temp = 0;
	temp = (uint8_t)strtol(bfr,&end,16);
	*b = temp;
	if(res >= 2) return 0; else return -1;
}


void g510FlushRx(void)
{
	usart_flush(G510_USART);
}


int g510Creg(void){
    
    char buf[50];

    g510FlushRx();
    g510TxCmd("AT+CREG?\r");

    g510ReadResp(buf,50,G510_RESPONSE_TIMEOUT);

    //xprintf("CREG response: %s\n",buf);

    char* ptr = strchr(buf,',');

    ptr++;

    return *ptr - '0';
}

int g510Cgatt(void){
    
    char buf[50];

    g510FlushRx();
    g510TxCmd("AT+CGATT?\r");

    g510ReadResp(buf,50,G510_RESPONSE_TIMEOUT);

    char* ptr = strchr(buf,':');

    ptr++;

    return *ptr - '0';
}

int g510Gtset(void){
    
    g510FlushRx();
    g510TxCmd("AT+gtset=IPRFMT,5\r");

    if(g510WaitForResponse("OK",G510_RESPONSE_TIMEOUT)) return -1;

    return 0;
}


int g510WaitForRegistration(int retries)
{
	int creg;
	while(retries--)
	{
		creg = g510Creg();

		if(creg==1) break;
		if(creg==5) break;

		vTaskDelay(1000);
	}


	if(retries==0) return -1; else return 0;
}

int g510ReadChunk(char* buf, int chunk_len, int offs, int ignorenewline, int* actuallen)
{
	//xprintf("g510ReadChunk!\n");
	*actuallen = 0;
	int res = 0;
	sprintf(buf,"AT+HTTPREAD=%d,%d\r",offs,chunk_len);

	g510TxCmd(buf);

	sprintf(buf,"+HTTPREAD: %d",chunk_len);

	//xprintf("waiting for response...\n");

	if(g510WaitForResponse(buf,2000)) {res=-6; xprintf("length does not match\n"); delay_ms(1000); goto DEAL_WITH_ERROR;}

	//xprintf("waiting for 0x0A...\n");

	g510ReadUntilChar(0x0A);

	char *ptr = buf;

	for(int i=0;i<chunk_len;i++)
	{
		uint8_t chr;
		if(usart_waitkey(G510_USART,(uint8_t*)&chr,500) == 0)
		{
			//xprintf("g510ReadChunk read timeout\n");
			vTaskDelay(1000);
			res = -123;
			goto DEAL_WITH_ERROR;
		}
		//*ptr = tolower(*ptr);
		if( !(ignorenewline && (chr==0x0A)) )
		{
			*ptr++ = chr;
			*actuallen = *actuallen + 1;
		}

	}

	return 0;

	DEAL_WITH_ERROR:

	//xprintf("g510ReadChunk error %d\n",res);

	vTaskDelay(1000);

	return res;
}


int g510ReadHeader(char* buf, int availData, int* pos, int* dataLen)
{
	*pos = *pos;
	*dataLen = *dataLen;
	int res = 0;
	xprintf("reading data header...\n");

	int actuallen;

	if( g510ReadChunk(buf,availData,0,0,&actuallen) ) {xprintf("g510ReadHeader call to g510ReadChunk returned error\n"); vTaskDelay(1000); res = -1; goto DEAL_WITH_ERROR;}

	buf[actuallen] = 0;

	char *ptr;

	const char* CONTENT_LENGTH_STR = "Content-Length: ";

	ptr = strstr(buf,CONTENT_LENGTH_STR);

	ptr+= strlen(CONTENT_LENGTH_STR);

	//xprintf("contlen=%s\n",ptr);

	if(ptr == NULL) {xprintf("Content-Length: could not be found\n"); res = -99; goto DEAL_WITH_ERROR;}

	*dataLen = strtol((char*)ptr,(char**)&ptr,10);

	//xprintf("content length decoded = %d\n",*dataLen);

	ptr = strstr(ptr,"\r\n\r\n");

	if(ptr == NULL) {xprintf("two newlinez not found\n"); res = -98; goto DEAL_WITH_ERROR;}

	ptr+=4;

	uint32_t tmpoffs = (uint32_t)ptr - (uint32_t)buf;

	*pos = tmpoffs;

	//xprintf("offset in payload: %d\n",*pos);

	//xprintf("\nin text format: %s\n",ptr);

	return 0;


	DEAL_WITH_ERROR:

	//xprintf("g510ReadHeader error %d\n",res);

	vTaskDelay(1000);

	return res;

}


int loadFile(const char* url, char* buf, uint32_t buf_len)
{
	//xprintf("loading to flash from URL: %s\n",url);
	int res;
	int retries = 2;

	if(g510WaitForRegistration(300))
	{
		return -1;
	}

	do
	{
		//xprintf("retries left %d...\n",retries);
		g510Creg();

		res = g510DownloadFileToBuf(url,buf,buf_len);

		vTaskDelay(1000);

		g510WaitForRegistration(100);

	}while( (res > -10) && (res < 0) && (retries--) );

	//xprintf("g510DownloadFileToBuf returned %d\n",res);

	return res;
}


void closeConn(void){
	g510TxCmd("AT+MIPCALL=0\r");
}



int g510DownloadFileToBuf(const char* address, char* buf, uint32_t max_len)
{
	int res = 0;

	/*g510TxCmd("AT+CGDCONT=1,\"IP\",\"" APN "\",\"0.0.0.0\",0,0\r");
	if(g510WaitForResponse("OK",30000)) return -1;*/

	vTaskDelay(1000);

	g510FlushRx();

	g510TxCmd("AT+MIPCALL=1,\"" APN "\",\"" USER "\",\"" PASSWD "\"\r");

	g510ReadResp(buf,1024,30000);

	//xprintf("mipcal response: %s\n",buf);

	if(strstr(buf,"OK")==0)
	{
		//xprintf("could not issue MIPCALL\n");
		return -2;
	}

	if(g510WaitForResponse("+MIPCALL:",30000)) return -3;

	g510FlushRx();

	/*sprintf(tempBuf,"AT+MIPOPEN=1,0,\"%s\",%d,0\r",address,port);

	g510TxCmd(tempBuf);

	if(g510WaitForResponse("+MIPOPEN",30000)) return -4;*/

	sprintf(buf,"AT+HTTPSET=\"URL\",\"%s\"\r",address);

	g510TxCmd(buf);

	if(g510WaitForResponse("OK",30000)) {closeConn(); return -4;}

	g510TxCmd("AT+HTTPSET=\"UAGENT\",\"Bootloader\"\r");

	if(g510WaitForResponse("OK",30000)) {closeConn(); return -5;}

	/*g510TxCmd("AT+HTTPSET=\"CONTYPE\",\"BINARY\"\r");

	if(g510WaitForResponse("OK",30000)) return -3;*/

	g510TxCmd("AT+HTTPACT=0\r");

	if(g510WaitForResponse("OK",30000)) {closeConn(); return -5;}

	if(g510WaitForResponse("+HTTP:",30000)) {closeConn(); return -5;}

	if(g510WaitForResponse("+HTTPRES:",60000)) {closeConn(); return -5;}

	g510ReadResp(buf,max_len,10000);

	//xprintf("http response: %s\n",buf);

	if( strstr(buf,",<404>,") )
	{
		return -404;
	}

	char* ptr;

	ptr = strchr(buf,'<');
	ptr++;
	ptr = strchr(ptr,'<');
	ptr++;
	ptr = strchr(ptr,'<');

	ptr++;

	int len = strtol(ptr,&ptr,10);

	//xprintf("decoded len=%d\n",len);

	int offs = 0;
	int payload_len = 0;

	if( g510ReadHeader(buf,len,&offs,&payload_len) ) {res = -8; xprintf("read header error\n"); vTaskDelay(1000); closeConn(); return res;}

	//xprintf("offs=%d, payload_len=%d\n",offs,payload_len);

	memmove(buf,&buf[offs],payload_len);

	buf[payload_len] = 0;

	//xprintf("buf at end: %s\n",buf);

	g510TxCmd("AT+MIPCALL=0\r");

	return 0;
}

int getSMS(char* buf, uint32_t max_len){
	if(g510WaitForRegistration(300))
	{
		return -1;
	}
	g510FlushRx();
	g510TxCmd("AT+CMGF=1\r");
	if(g510WaitForResponse("OK",1000)) return -2;
	g510TxCmd("AT+CPMS=\"SM\",\"SM\",\"SM\"\r");
	g510ReadResp(buf,1024,30000);
	//xprintf("cmps response: %s\n",buf);

	g510TxCmd("AT+CMGL=\"ALL\"\r");
	g510ReadResp(buf,1024,30000);
	xprintf("cmps response: %s\n",buf);

	return 0;

}

int sendSMS(char* buf, uint32_t max_len){
	if(g510WaitForRegistration(300))
	{
		return -1;
	}
	g510FlushRx();
	g510TxCmd("AT+CMGF=1\r");
	if(g510WaitForResponse("OK",1000)) return -2;
	// g510TxCmd("AT+CSMP=17,167,0,0");
	// g510ReadResp(buf,1024,30000);
	// xprintf("csmp response: %s\n",buf);



	g510TxCmd("AT+CMGS=\"+48500615557\"\r"); //
	g510ReadResp(buf,1024,30000);
	//xprintf("cmgs response: %s\n",buf);

	g510TxCmd("Siema, tutaj modemik\032"); //500615557
	g510ReadResp(buf,1024,30000);
	//xprintf("cmgs response: %s\n",buf);

	return 0;
}

int g510_Mipcall(void){
    if(g510WaitForRegistration(300)) return -1;
    
    g510FlushRx();
    g510TxCmd("AT+MIPCALL=1,\"" APN "\",\"" USER "\",\"" PASSWD "\"\r");
    if(g510WaitForResponse("OK",30000)) return 0;
    
    char buf[1024];
    g510ReadResp(buf,1024,30000);
    xprintf("soemthing went wrong %s\n", buf);
    return -1;

}

int g510Start(void){    
    int result = g510PowerOn();
    if(result){
        xprintf("g510PowerOn error: %d", result);
        return -1;
    }
    result = g510Creg();
    if(result != 1 && result != 5){ 
        xprintf("g510Creg error: %d, device not registered\n", result);
        return -2; 
    }
    result = g510Cgatt();
    if(result == 0){ 
        xprintf("g510Cgatt error: %d, device not attached\n", result);
        return -3; 
    }
    /*
    result = g510Gtset(void);
    if(result){
        xprintf("g510Gtset error: %d, cannot set cache mode\n", result);
        return -4;
    } 
    */   
    
    result = g510_Mipcall();
    if(result){
        xprintf("g510Mipcall error: %d, device cannot connect\n", result);
        return -5;
    }
    return 0;
}