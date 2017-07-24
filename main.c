#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "usart.h"
#include "board.h"
#include "sd/sd.h"
#include "fatfs/ff.h"
#include "flash.h"

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

void * lwm2m_connect_server(uint16_t secObjInstID,
        void * userData) {
    client_data_t * dataP;
    char * uri;
    char * host;
    char * port;

    connection_t * newConnP = NULL;

    dataP = (client_data_t *) userData;

    uri = get_server_uri(dataP->securityObjP, secObjInstID);


    if (uri == NULL) return NULL;

    fprintf(stdout, "Connecting to %s instance: %d\r\n", uri, secObjInstID);

    // parse uri in the form "coaps://[host]:[port]"
    if (0 == strncmp(uri, "coaps://", strlen("coaps://"))) {
        host = uri + strlen("coaps://");
    } else if (0 == strncmp(uri, "coap://", strlen("coap://"))) {
        host = uri + strlen("coap://");
    } else {
        goto exit;
    }
    port = strrchr(host, ':');
    if (port == NULL) goto exit;
    // remove brackets
    if (host[0] == '[') {
        host++;
        if (*(port - 1) == ']') {
            *(port - 1) = 0;
        } else goto exit;
    }
    // split strings
    *port = 0;
    port++;
    newConnP = connection_create(dataP->connList, dataP->sock, host, port);
    if (newConnP == NULL) {
        fprintf(stderr, "Connection creation failed.\r\n");
    } else {
        dataP->connList = newConnP;
    }

exit:
    lwm2m_free(uri);
    return (void *) newConnP;
}

void lwm2m_close_connection(void * sessionH,
        void * userData) {
    client_data_t * app_data;
    connection_t * targetP;

    app_data = (client_data_t *) userData;
    targetP = (connection_t *) sessionH;

    if (targetP == app_data->connList) {
        app_data->connList = targetP->next;
        lwm2m_free(targetP);
    } else {
        connection_t * parentP;

        parentP = app_data->connList;
        while (parentP != NULL && parentP->next != targetP) {
            parentP = parentP->next;
        }
        if (parentP != NULL) {
            parentP->next = targetP->next;
            lwm2m_free(targetP);
        }
    }
}


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

void taskMain(void *params)
{
    xprintf("hello from the main task!\n");
    int result;
    uint8_t socket;

    //brdAuxSupply(1);
    //brdLcdSupply(1);
    //brdRfidSupply(1);

    g510Init();
    result = g510Start();
    if(result){
        xprintf("g510Start error %d", result);
        // end task
    }
    socket = g510_openSocket();
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

        objArray[3] = get_object_firmware(&lwm2mContext);
        if (NULL == objArray[5]) {
            xprintf("Failed to create Firmware object\r\n");
            //return -1;
            quit = 1;
        }

        objArray[4] = get_object_test();
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

            tv.tv_sec = SELECT_TIMEOUT; 

            result = lwm2m_step(lwm2mContext, &(tv.tv_sec));
            if (result != 0) {
                fprintf(stderr, "lwm2m_step() failed: 0x%X\r\n", result);
                //return -1;
                break;
            }

            char buffer[PACKET_SIZE];
            int numBytes;

            /*
             * We retrieve the data received
             */
            numBytes = g510_udpRead(buffer, data.sock, SERVER_IP_STR, SERVER_PORT_STR);

            if (numBytes == 0) { // connections are dirty a little bit fix it later
                vTaskDelay(SELECT_TIMEOUT); // nothing to read -> sleep some time
            } 
            else if (0 < numBytes) { // msg from server
                
                connection_t * connP;
                connP = connection_find(data.connList, SERVER_IP_STR, SERVER_PORT_STR);
                if (connP != NULL) {
                    /*
                     * Let liblwm2m respond to the query depending on the context
                     */
                    lwm2m_handle_packet(lwm2mContext, buffer, numBytes, connP);
                }             
            } 
            else{
                /*
                 * This packet comes from an unknown peer or error 
                 */
                fprintf(stderr, "some error\r\n");
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
        free_object_firmware(objArray[3]);
        free_object_test(objArray[4]);

        xprintf("wakaama has finised\r\n");
    }
    
    g510_closeSocket(socket);
    closeConn();
    g510PowerOff();
    
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
