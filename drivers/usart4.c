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
#include "usart4.h"

#define USARTx				UART4
#define USARTx_IRQn			UART4_IRQn
#define GPIOx_TX			GPIOC
#define PIN_TX				GPIO_Pin_10
#define GPIOx_RX			GPIOC
#define PIN_RX				GPIO_Pin_11
#define RX_QUEUE_LEN		50
#define TX_QUEUE_LEN		10

static xQueueHandle usartTxQueue=NULL,usartRxQueue=NULL;
static xSemaphoreHandle txReadySemaphore;

static void usartTxTask(void *params);

//high=tx, low=rx
#define DIR_TX	1
#define DIR_RX	0
static inline void dirOutput(int hi)
{
	//if(hi) GPIOx_RTS->BSRR = PIN_RTS; else GPIOx_RTS->BRR = PIN_RTS;
}

void usart4_shdn(uint32_t shdn_on)
{
	if(shdn_on)
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, DISABLE);
	else
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
}


int usart4_txt(const char * txt)
{
	int good;
	
	while(*txt)
	{
		good = usart4_chr(*txt++);
		if(good==0) return 0;
	}
	return 1;
}

int usart4_chr(char chr)
{
	if(xQueueSend(usartTxQueue,&chr,1000)==pdTRUE)
	{
		return(1);
	}
	else
	{
		xprintf("usart4_chr: FAIL\n");
		return(0);
	}
}

int usart4_waitkey(uint8_t* key, portTickType timeout)
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

char usart4_inkey(void)
{
	char chr;
	if(xQueueReceive(usartRxQueue,&chr,0)==pdTRUE)
	{
		return(chr);
	}
	return(0);
}

void usart4Init(uint32_t baud)
{
	portENTER_CRITICAL();
	{
		
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
		
		GPIO_Init(GPIOC, &(GPIO_InitTypeDef) {
			.GPIO_Pin   = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP
		});
		
		GPIO_Init(GPIOC, &(GPIO_InitTypeDef) {
			.GPIO_Pin = GPIO_Pin_11,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_PuPd = GPIO_PuPd_DOWN
		});
		
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);
		
		USART_Init(USARTx, &(USART_InitTypeDef) {
			.USART_BaudRate = baud,
			.USART_WordLength = USART_WordLength_8b,
			.USART_StopBits = USART_StopBits_1,
			.USART_Parity = USART_Parity_No ,
			.USART_HardwareFlowControl = USART_HardwareFlowControl_None,
			.USART_Mode = USART_Mode_Rx | USART_Mode_Tx
		});
		
		NVIC_Init(&(NVIC_InitTypeDef) {
			.NVIC_IRQChannel = USARTx_IRQn,
			.NVIC_IRQChannelPreemptionPriority = 12,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE
		});
		
		USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
		USART_Cmd(USARTx, ENABLE);
		
		xprintf("USART4 HW initialized\n");
		
		usartTxQueue = xQueueCreate(TX_QUEUE_LEN,sizeof(char));
		usartRxQueue = xQueueCreate(RX_QUEUE_LEN,sizeof(char));
		
		if(usartTxQueue==NULL) xprintf("USART4 TX Queue init error!\n"); else xprintf("USART4 TX Queue init OK\n");
		if(usartRxQueue==NULL) xprintf("USART4 RX Queue init error!\n"); else xprintf("USART4 RX Queue init OK\n");
		
		xTaskCreate(usartTxTask,NULL,configMINIMAL_STACK_SIZE,NULL,1,NULL);
		
		vSemaphoreCreateBinary(txReadySemaphore);
		
		if(txReadySemaphore==NULL) xprintf("USART4 TX Ready Semaphore init error!\n"); else xprintf("USART TX Ready Semaphore init OK\n");
	}
	portEXIT_CRITICAL();
	
}

static void usartTxTask(void *params)
{
	if(xSemaphoreTake(txReadySemaphore,0)!=pdTRUE) xprintf("usartTxTask (USART4): tx semphr not obtained at start!");
	
	while(1)
	{
		char chrToTx;
		if(xQueueReceive(usartTxQueue,&chrToTx,portMAX_DELAY))
		{
			if(xSemaphoreTake(txReadySemaphore,500)!=pdTRUE) xprintf("usartTxTask (USART4): tx semphr not obtained after TX!");
			dirOutput(DIR_TX);
			USART_SendData(USARTx, (uint16_t)chrToTx);
			USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
		}
	}
}


void UART4_IRQHandler(void)
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
		xSemaphoreGiveFromISR(txReadySemaphore,&xHigherPriorityTaskWoken);
		USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
		USART_ITConfig(USARTx, USART_IT_TC, ENABLE);
		USART_ClearITPendingBit(USARTx, USART_IT_TXE);
	}
	
	if(USART_GetITStatus(USARTx, USART_IT_TC) != RESET)
	{
		//debug_chr('c');
		dirOutput(DIR_RX);
		USART_ClearITPendingBit(USARTx, USART_IT_TC);
		USART_ITConfig(USARTx, USART_IT_TC, DISABLE);
	}
	
	if( xHigherPriorityTaskWoken )
	{
		portYIELD();
	}
}

