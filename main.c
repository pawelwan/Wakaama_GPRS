#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "usart.h"
#include "board.h"
#include "g510_frt.h"
#include "sd/sd.h"
#include "fatfs/ff.h"
#include "flash.h"
#include "g510_socket.h"



#define FILE_BUF_SIZE	2048
// static FATFS fs;
// static FIL file;
// static char fileBuffer[FILE_BUF_SIZE];

#ifndef UNIQUE_ID
#warning unique id not defined!
const unsigned int UNIQUE_ID[3] = {0xCB88FF33,0x32395331, 0x57148018};
#endif


void init(void)
{
    //RCC - Reset and Clock Control
    //enable clocks to GPIOs
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    //GPIO and USART configuration
    debug_init_default();
    xprintf("\n*** reset ***\nSTM32F4 + G510 Test Project\n" );
    printf("This is normal printf test!\r\n");

  //clock status
	RCC_ClocksTypeDef RCC_ClocksStatus;
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	xprintf("Clock info: F_APB1 = %d Hz, F_APB2 = %d Hz\n",(int)RCC_ClocksStatus.PCLK1_Frequency,(int)RCC_ClocksStatus.PCLK2_Frequency);

    //NVIC - Nested Vectored Interrupt Controller
    //Group priority configuration
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	brdInit();

}

#define BUF_SIZE		1000
static char buf[BUF_SIZE];

void taskMain(void *params)
{
	xprintf("hello from the main task!\n");

	//brdAuxSupply(1);
	//brdLcdSupply(1);
	//brdRfidSupply(1);

	g510Init();

	while(1)
	{

		//xprintf("x");
		vTaskDelay(500);
		char key = debug_inkey();

		led1(LED_TOGGLE);

		if(key)
		{
			switch(key)
			{
				case '0':
				case '1':
				{
					uint8_t in = key - '0';
					xprintf("siema\nInput %d state = %d\n",in,brdReadOdInput(in));
					break;
				}
				case 'L':
				{
					//g510PowerOn();
					loadFile("http://home.agh.edu.pl/~rabw/comm_test/contents.txt",buf,BUF_SIZE);
					xprintf("buf contents: %s\n",buf);
					//g510PowerOff();
					break;
				}

				case 'x':
				{
					g510PowerOn();
					xprintf("started\n");
					break;
				}

				case 'y':
				{
					getSMS(buf,BUF_SIZE);
					xprintf("buf contents: %s\n",buf);
					break;
				}

				case 'z':
				{
					g510PowerOff();
					xprintf("stopped\n");
					break;
				}

				case 'r':
				{
					xprintf("%d\n", g510_socket());
					//g510_getIP();
					break;
				}
				case 'f':
				{
					uint8_t buf[] = {1, 2, 3, 4, 5, 6, 7, 8};
					FLASH_Status res = flash_program(FLASH_SECTOR_3_ADDR, buf,  8);
					xprintf("%d\n", res);
					break;
				}
				default:
					xprintf("nie rozpoznane polecenie\n");
					break;
			}
		}
	}

}


int main(void)
{
	init();

	int res = xTaskCreate(taskMain,NULL,8000,NULL,1,NULL);
	xprintf("taskMain xTaskCreate returned %d\n",res);

	/*res = xTaskCreate(taskUsb,NULL,2000,NULL,1,NULL);
	xprintf("taskUsb xTaskCreate returned %d\n",res);*/

    vTaskStartScheduler();

    return 0;
}
