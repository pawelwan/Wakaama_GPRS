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
#include "usart2.h"

#define USARTx				USART2
#define USARTirq			USART2_IRQn
#define GPIOx_TX			GPIOD
#define PIN_TX				GPIO_Pin_5
#define GPIOx_RX			GPIOD
#define PIN_RX				GPIO_Pin_6
#define RX_QUEUE_LEN		50
#define TX_QUEUE_LEN		10
#define GPIOx_RTS			GPIOD
#define PIN_RTS				GPIO_Pin_4
#define GPIOx_RXEN			GPIOD
#define PIN_RXEN			GPIO_Pin_3


static xQueueHandle usartTxQueue=NULL,usartRxQueue=NULL;
static xSemaphoreHandle txReadySemaphore;

static void usartTxTask(void *params);

//high=tx, low=rx
#define DIR_TX	1
#define DIR_RX	0
#define DIR_SHDN	2
static inline void dirOutput(int opt)
{
	switch(opt)
	{
		case DIR_TX:
		{
			GPIO_SetBits(GPIOx_RTS,PIN_RTS);
			GPIO_SetBits(GPIOx_RXEN,PIN_RXEN);
			break;
		}
		case DIR_RX:
		{
			GPIO_ResetBits(GPIOx_RTS,PIN_RTS);
			GPIO_ResetBits(GPIOx_RXEN,PIN_RXEN);
			break;
		}
		case DIR_SHDN:
		{
			GPIO_ResetBits(GPIOx_RTS,PIN_RTS);
			GPIO_SetBits(GPIOx_RXEN,PIN_RXEN);
			break;
		}
	}
	//if(hi) GPIOx_RTS->BSRR = PIN_RTS; else GPIOx_RTS->BRR = PIN_RTS;
}



int usart2_txt(const char * txt)
{
	int good;
	
	while(*txt)
	{
		good = usart2_chr(*txt++);
		if(good==0) return 0;
	}
	return 1;
}

int usart2_chr(char chr)
{
	if(xQueueSend(usartTxQueue,&chr,1000)==pdTRUE)
	{
		return(1);
	}
	else
	{
		xprintf("usart2_chr: FAIL\n");
		return(0);
	}
}

int usart2_waitkey(uint8_t* key, portTickType timeout)
{
	if(xQueueReceive(usartRxQueue,key,timeout)==pdTRUE)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}

char usart2_inkey(void)
{
	char chr;
	if(xQueueReceive(usartRxQueue,&chr,0)==pdTRUE)
	{
		return(chr);
	}
	return(0);
}

void usart2_shdn(uint32_t shdn_on)
{
	if(shdn_on)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, DISABLE);
		dirOutput(DIR_SHDN);
	}
	else
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		dirOutput(DIR_RX);
	}
}

void usart2Init(uint32_t baud)
{
	portENTER_CRITICAL();
	{
		
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		
		GPIO_Init(GPIOx_TX, &(GPIO_InitTypeDef) {
			.GPIO_Pin   = PIN_TX,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP
		});
		
		GPIO_Init(GPIOx_RX, &(GPIO_InitTypeDef) {
			.GPIO_Pin = PIN_RX,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_PuPd = GPIO_PuPd_UP
		});
		
		GPIO_Init(GPIOx_RTS, &(GPIO_InitTypeDef) {
			.GPIO_Pin   = PIN_RTS,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode  = GPIO_Mode_OUT,
			.GPIO_PuPd  = GPIO_PuPd_NOPULL,
			.GPIO_OType = GPIO_OType_PP
		});
		
		GPIO_Init(GPIOx_RXEN, &(GPIO_InitTypeDef) {
			.GPIO_Pin   = PIN_RXEN,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode  = GPIO_Mode_OUT,
			.GPIO_PuPd  = GPIO_PuPd_NOPULL,
			.GPIO_OType = GPIO_OType_PP
		});
		
		dirOutput(DIR_RX);
		
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
		
		USART_Init(USART2, &(USART_InitTypeDef) {
			.USART_BaudRate = baud,
			.USART_WordLength = USART_WordLength_8b,
			.USART_StopBits = USART_StopBits_1,
			.USART_Parity = USART_Parity_No ,
			.USART_HardwareFlowControl = USART_HardwareFlowControl_None,
			.USART_Mode = USART_Mode_Rx | USART_Mode_Tx
		});
		
		NVIC_Init(&(NVIC_InitTypeDef) {
			.NVIC_IRQChannel = USART2_IRQn,
			.NVIC_IRQChannelPreemptionPriority = 12,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE
		});
		
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
		USART_Cmd(USART2, ENABLE);
		
		xprintf("USART2 HW initialized\n");
		
		usartTxQueue = xQueueCreate(TX_QUEUE_LEN,sizeof(char));
		usartRxQueue = xQueueCreate(RX_QUEUE_LEN,sizeof(char));
		
		if(usartTxQueue==NULL) xprintf("USART2 TX Queue init error!\n"); else xprintf("USART2 TX Queue init OK\n");
		if(usartRxQueue==NULL) xprintf("USART2 RX Queue init error!\n"); else xprintf("USART2 RX Queue init OK\n");
		
		xTaskCreate(usartTxTask,NULL,configMINIMAL_STACK_SIZE,NULL,1,NULL);
		
		vSemaphoreCreateBinary(txReadySemaphore);
		
		if(txReadySemaphore==NULL) xprintf("USART2 TX Ready Semaphore init error!\n"); else xprintf("USART TX Ready Semaphore init OK\n");
	}
	portEXIT_CRITICAL();
	
}

static void usartTxTask(void *params)
{
	//if(xSemaphoreTake(txReadySemaphore,0)!=pdTRUE) xprintf("usartTxTask (USART2): tx semphr not obtained at start!");
	
	while(1)
	{
		char chrToTx;
		if(xQueueReceive(usartTxQueue,&chrToTx,portMAX_DELAY))
		{
			if(xSemaphoreTake(txReadySemaphore,500)!=pdTRUE) xprintf("usartTxTask (USART2): tx semphr not obtained after TX!");
			dirOutput(DIR_TX);
			USART_SendData(USARTx, (uint16_t)chrToTx);
			USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
		}
	}
}


void USART2_IRQHandler(void)
{
	//debug_chr('i');
	portBASE_TYPE xHigherPriorityTaskWoken=pdFALSE;
	
	//xprintf("irqstatus=%x\n",USART_GetITStatus(USARTx, USART_IT_RXNE));
	
	if(USART_GetITStatus(USARTx, USART_IT_RXNE) != RESET)
	{
		//debug_chr('r');
		char chr = USART_ReceiveData(USARTx);
		xQueueSendFromISR(usartRxQueue,&chr,&xHigherPriorityTaskWoken);
		USART_ClearITPendingBit(USARTx, USART_IT_RXNE);
	}
	
	if(USART_GetITStatus(USARTx, USART_IT_TXE) != RESET)
	{
		//debug_chr('e');
		USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
		xSemaphoreGiveFromISR(txReadySemaphore,&xHigherPriorityTaskWoken);
		USART_ITConfig(USARTx, USART_IT_TC, ENABLE);
		USART_ClearITPendingBit(USARTx, USART_IT_TXE);
	}
	
	if(USART_GetITStatus(USARTx, USART_IT_TC) != RESET)
	{
		//debug_chr('c');
		dirOutput(DIR_RX);
		USART_ITConfig(USARTx, USART_IT_TC, DISABLE);
		USART_ClearITPendingBit(USARTx, USART_IT_TC);
	}
	
	if( xHigherPriorityTaskWoken )
	{
		portYIELD();
	}
}

