/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "firmware_update.h"
#include "wakaama/client/config_client.h"
#include "wakaama/internals.h"

size_t binFileCurrSize;
const char * header;

static void printErr(char * error, coap_packet_t * response){
    xprintf("\r\n\tERROR %s\r\n", error);
    xprintf("\t DOWNLOADING FAILED\r\n");
    lwm2m_free(response->payload);
}

static void printErrArg(char * error, int arg, coap_packet_t * response){
    xprintf("\r\n\tERROR %s %d\r\n", error, arg);
    xprintf("\t DOWNLOADING FAILED\r\n");
    lwm2m_free(response->payload);
}

static int initRequest(coap_packet_t * request, const char * binUri, lwm2m_context_t * ContextP, 
        uint32_t block_num, uint16_t block_size){
    //set get packet
    coap_init_message(request, COAP_TYPE_CON, COAP_GET, ContextP->nextMID++);

    //set token
    coap_set_header_token(request,0xAF4, sizeof(0xFF));

    //set block
    if(coap_set_header_block2(request, block_num, (uint8_t)0, block_size) == 0){
        printErr("coap_set_header_block2 inappropirate size or too high packet number", request);
        return -1;
    }

    //set resource
    coap_set_header_uri_path(request, binUri);

    //set payload
    coap_set_payload(request, NULL, 0);
    
    #ifdef COAPLOG
    xprintf("Type: %u Mid: [%u], GET, %s, 2:%d/%u/%u \r\n",
    request->type, request->mid, request->uri_path->data,
    request->block2_num, request->block2_more, request->block2_size);
    #endif
    return 0;
}

int downloadFirmware(const char * uri, lwm2m_context_t * ContextP){

    client_data_t * dataP = (client_data_t *)ContextP->userData;
    uint8_t retrys = 0;
    uint32_t block_num = 0;
    uint16_t block_size = REST_MAX_CHUNK_SIZE;
    binFileCurrSize = 0;
    uint32_t expBinFileSize = 0;
    uint32_t header_curr_size = 0;
    uint8_t responseBuffer[sizeof(uint8_t) *PACKET_SIZE];
    uint8_t progress = 0;
    FirmwareHeader_t header;

    static coap_packet_t request[1];
    static coap_packet_t response[1];
    
    const char * binUri = (strrchr(uri,'/'));
    binUri++;
    
    fprintf(stdout, "\r\n\t DOWNLOADING UPDATE\r\n");          
    
    while(1){
        
        //clean mail box
        while(g510_udpRead(responseBuffer, dataP->sock, SERVER_IP_STR, SERVER_PORT_STR));
        
        coap_status_t coap_error_code = NO_ERROR;
        //prepare request
        if(initRequest(request, binUri, ContextP, block_num, block_size) != 0){
            printErr("initRequest", request);
            return -1;
        }

        //send request
        coap_error_code = message_send(ContextP ,request, dataP->connList);
        if (coap_error_code != COAP_IGNORE && coap_error_code != 0){
            printErrArg("message_send", coap_error_code, response);
            return -1;
        }
        
        /*##################################################################*/
        /*                            RECEIVE PACKET                        */
        /*##################################################################*/
        int i = 5;
        int numBytes;
        
        while(i > 0){
           
            numBytes = g510_udpRead(responseBuffer, dataP->sock, SERVER_IP_STR, SERVER_PORT_STR);
            
            if (numBytes == 0) { // connections are dirty a little bit fix it later
                vTaskDelay(COAP_RESPONSE_TIMEOUT); // nothing to read -> sleep some time
                continue;
            } 
            else if (0 < numBytes) { // msg from server
                
                connection_t * connP;
                connP = connection_find(dataP->connList, SERVER_IP_STR, SERVER_PORT_STR);
                if (connP != NULL) {
                    break;                  
                }             
            } 
            else{ // packet from different source, reject
                fprintf(stderr, "received bytes ignored!\r\n");
                continue;
            }
            i--;
        }
        if(i == 0){
        
            retrys++;
            if(retrys > 5){
                printErr("server is not responding",  response);
                return -1;
            }
            xprintf("\t RETRY DOWNLOADING\r\n");
            continue;
        }
                  
        uint32_t block2_num = 0;
        uint16_t block2_size = REST_MAX_CHUNK_SIZE;
        uint8_t block2_more = 0;
        //check if response is correct 
        coap_error_code = coap_parse_message(response, responseBuffer, (uint16_t)numBytes);
        if (coap_error_code != COAP_IGNORE && coap_error_code != 0){
            retrys++;
            if(retrys > 5){
                printErrArg("coap_parse_message", coap_error_code,  response);
                return -1;
            }
            xprintf("\t RETRY DOWNLOADING\r\n");
            continue;
        }
        #ifdef COAPLOG
        xprintf("Type: %u Mid: [%u], 2.05 content, 2:%d/%u/%u \r\n",
        response->type, response->mid,
        response->block2_num, response->block2_more, response->block2_size);
        #endif                
        //setup for next request
        if (coap_get_header_block2(response, &block2_num, &block2_more, &block2_size, NULL)){
            block_size = MIN(block2_size, REST_MAX_CHUNK_SIZE);

            if(block2_num != block_num){
                retrys++;
                if(retrys > 5){
                    printErrArg("wrong packet received, expected %d", block_num,  response);
                    return -1;
                }
                xprintf("\t RETRY DOWNLOADING\r\n");
                continue;
            }
            block_num++; // received good packet, ask for next
        }
        #ifdef COAPLOG
        xprintf("Payload Size:%d\r\n", response->payload_len);
        #endif

        // process header
        uint16_t remaining_header_len = 0;
        if(header_curr_size < HEADER_SIZE){

            remaining_header_len =
                    (response->payload_len <= HEADER_SIZE - header_curr_size) ?
                    response->payload_len:
                    HEADER_SIZE - header_curr_size;
            memcpy(((void*)&header)+ header_curr_size, response->payload, remaining_header_len);
            header_curr_size += remaining_header_len;
            response->payload_len -= remaining_header_len;
            expBinFileSize = header.size;

            if(response->payload_len <= 0){ // end of  packet?
                lwm2m_free(response->payload);
                response->payload = NULL;
                response->payload_len = 0;
                expBinFileSize = header.size;
                continue; // next packet, header not finished
            }
        }

        // MOVE MEMORY HERE

        if(upload_firmware_batch(&response->payload[remaining_header_len], response->payload_len) != 1){
            printErr("upload_firmware_batch",  response);
            return -1;
        }

        // MOVE MEMORY ABOVE

        binFileCurrSize += response->payload_len;

        lwm2m_free(response->payload);
        response->payload = NULL;
        response->payload_len = 0;

        if((100*binFileCurrSize)/expBinFileSize > progress){
            progress = (100*binFileCurrSize)/expBinFileSize;
            xprintf("Downloaded %d%c\r\n", progress, 37, block2_more);
        }


        //check end condition // received last response
        if(block2_more == 0){

            xprintf("\r\nTotal binPackage size:%d, expected:%d\r\n", binFileCurrSize, expBinFileSize);
            if(expBinFileSize !=  binFileCurrSize){
                printErr("invalide binary size!", response);
                return -1;
            }

            // checksum
            if(check_firmware(header.crc) != 1){
                printErr("CRC does not match", response);
            }
            fprintf(stdout, "\n\t DOWNLOAD COMPLETED\r\n");
            //NVIC_SystemReset(); //need core_cm4.h to be included
            break; // last packet finished;
        }
        retrys = 0;
    }
    return 0;  
}