#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "usart.h"
#include "board.h"
#include "sd/sd.h"
#include "fatfs/ff.h"
#include "flash.h"

#include "g510_frt.h"
#include "g510_socket.h"

#include "wakaama/client/lwm2m_client.h"



#define FILE_BUF_SIZE	2048
// static FATFS fs;
// static FIL file;
// static char fileBuffer[FILE_BUF_SIZE];

#ifndef UNIQUE_ID
#warning unique id not defined!
const unsigned int UNIQUE_ID[3] = {0xCB88FF33,0x32395331, 0x57148018};
#endif

/*
* WAKAAMA VALUES START
*/
static lwm2m_context_t * lwm2mContext = NULL;
static int quit = 0; // usefull to controll flow of program
static lwm2m_object_t * objArray[OBJ_COUNT];
static client_data_t data;
/*
* WAKAAMA VALUES END
*/


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
    int result;
    char socket;

    //brdAuxSupply(1);
    //brdLcdSupply(1);
    //brdRfidSupply(1);

    g510Init();
    result = g510Start(void);
    if(result){
        xprintf("g510Start error %d", result);
        // end task
    }
    socket = g510_open_socket(void);
    if(socket > '4' || socket < '1'){
        xprintf("g510_open_socket error %u", socket);
        // end task
    }
    
    while(1){
        vTaskDelay(2000); // 2 seconds
        if(quit == 1){
            continue;
        }

        xprintf("wakaama has started!\n");
        result = 0;

        memset(&data, 0, sizeof (client_data_t));
        data.sock = socket;
        data.addressFamily = AF_INET;

        objArray[0] = get_object_security();
        if (NULL == objArray[0]) {
            xprintf("Failed to create security object\r\n");
            //return -1;
            quit = 1;
        }
        data.securityObjP = objArray[0];

        objArray[1] = get_object_server();
        if (NULL == objArray[1]) {
            xprintf("Failed to create server object\r\n");
            //return -1;
            quit = 1;
        }

        objArray[2] = get_object_device();
        if (NULL == objArray[2]) {
            xprintf("Failed to create Device object\r\n");
            //return -1;
            quit = 1;
        }

        objArray[3] = get_object_led();
        if (NULL == objArray[3]) {
            xprintf("Failed to create Led object\r\n");
            //return -1;
            quit = 1;
        }

        objArray[4] = get_object_accelerometer();
        if (NULL == objArray[4]) {
            xprintf("Failed to create Accelerometer object\r\n");
            //return -1;
            quit = 1;
        }

        objArray[5] = get_object_firmware(&lwm2mContext);
        if (NULL == objArray[5]) {
            xprintf("Failed to create Firmware object\r\n");
            //return -1;
            quit = 1;
        }

        objArray[6] = get_object_test();
        if (NULL == objArray[6]) {
            xprintf("Failed to create Test object\r\n");
            //return -1;
            quit = 1;
        }

        // init context
        lwm2mContext = lwm2m_init(&data);
        if (NULL == lwm2mContext) {
            xprintf("lwm2m_init() failed\r\n");
            //return -1;
            quit = 1;
        }
        // configure context
        result = lwm2m_configure(lwm2mContext, DEVICE_NAME, NULL, NULL, OBJ_COUNT, objArray);
        if (result != 0) {
            fprintf(stderr, "lwm2m_configure() failed: 0x%X\r\n", result);
            //return -1;
            quit = 1;
        }

        while (quit == 0) {
            struct timeval tv;  //TODO
            fd_set readfds;  //TODO

            tv.tv_sec = SELECT_TIMEOUT; 
            tv.tv_usec = 0;  //TODO

            FD_ZERO(&readfds); //TODO
            FD_SET(data.sock, &readfds);  //TODO

            /*
             * This function does two things:
             *  - first it does the work needed by liblwm2m (eg. (re)sending some packets).
             *  - Secondly it adjusts the timeout value (default 60s) depending on the state of the transaction
             *    (eg. retransmission) and the time before the next operation
             */
            result = lwm2m_step(lwm2mContext, &(tv.tv_sec));
            if (result != 0) {
                fprintf(stderr, "lwm2m_step() failed: 0x%X\r\n", result);
                //return -1;
                break;
            }
            /*
             * This part wait for an event on the socket until "tv" timed out (set
             * with the precedent function)
             */
            result = lwip_select(FD_SETSIZE, &readfds, NULL, NULL, &tv);  //TODO

            if (result < 0) {
                if (errno != EINTR) {
                    fprintf(stderr, "Error in select(): %d %s\r\n", errno, strerror(errno));
                }
            } else if (result > 0) {
                uint8_t buffer[PACKET_SIZE];
                int numBytes;

                /*
                 * If an event happens on the socket
                 */
                if (FD_ISSET(data.sock, &readfds)) {  //TODO
                    struct sockaddr addr;
                    socklen_t addrLen;

                    addrLen = sizeof (addr);

                    /*
                     * We retrieve the data received
                     */
                    numBytes = lwip_recvfrom(data.sock, buffer, PACKET_SIZE, 0, (struct sockaddr *) &addr, &addrLen);

                    if (0 > numBytes) {
                        fprintf(stderr, "Error in recvfrom(): %d %s\r\n", errno, strerror(errno));
                    } else if (0 < numBytes) {
                        connection_t * connP;

                        connP = connection_find(data.connList, &addr, addrLen);
                        if (connP != NULL) {
                            /*
                             * Let liblwm2m respond to the query depending on the context
                             */
                            lwm2m_handle_packet(lwm2mContext, buffer, numBytes, connP);
                        } else {
                            /*
                             * This packet comes from an unknown peer
                             */
                            fprintf(stderr, "received bytes ignored!\r\n");
                        }
                    }
                }
            }
        }

        /*
         * Finally when the loop is left, we unregister our client from it
         */
        lwm2m_close(lwm2mContext);
        connection_free(data.connList);


        free_object_security(objArray[0]);
        free_object_server(objArray[1]);
        free_object_device(objArray[2]);
        free_object_led(objArray[3]);
        free_object_accelerometer(objArray[4]);
        free_object_firmware(objArray[5]);
        free_object_test(objArray[6]);

        xprintf("wakaama has finised\r\n");
    }



    /*
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
     * */
    
    g510_close_socket(socket);
    closeConn(void);
    g510PowerOff(void);
    

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
