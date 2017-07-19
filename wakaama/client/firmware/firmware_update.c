/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "firmware_update.h"
#include "wakaama/client/connection.h"
#include "wakaama/client/config_client.h"
#include "wakaama/internals.h"

size_t binFileCurrSize;
const char * binFile; //TO DO

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
    binFile = mem_malloc(FIRMWARE_PACKAGE_SIZE); //TO DO
    if(binFile == NULL){
        xprintf("ERROR mem_malloc failed \r\n\n\t DOWNLOAD FAILED\r\n");
        // TO DO
    }
   
    static coap_packet_t request[1];
    static coap_packet_t response[1];
    
    const char * binUri = (strrchr(uri,'/'));
    binUri++;
    
    fprintf(stdout, "\r\n\t DOWNLOADING UPDATE\r\n");          
    
    while(1){
        
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
        
        struct timeval tv;
        fd_set readfds;
        int result;

        tv.tv_sec = COAP_RESPONSE_TIMEOUT;
        tv.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(dataP->sock, &readfds);
        // check if message came
        result = lwip_select(FD_SETSIZE, &readfds, NULL, NULL, &tv);

        if (result < 0){
            if (errno != EINTR){
                printErrArg("select", errno, response);
                return -1;
            }
            retrys++;
            if(retrys > 5){
                printErr("server is not responding",  response);
                return -1;
            }
            xprintf("\t RETRY DOWNLOADING\r\n");
            continue;
        }
        //got message
        else if (result > 0){
            uint8_t responseBuffer[sizeof(uint8_t) *PACKET_SIZE];
            int numBytes;
            // is our socket readable
            if (FD_ISSET(dataP->sock, &readfds)) { 
                struct sockaddr addr;
                socklen_t addrLen;
                addrLen = sizeof (addr);

                numBytes = lwip_recvfrom(dataP->sock, responseBuffer, PACKET_SIZE, 0, (struct sockaddr *) &addr, &addrLen);
                // received some bytes to process
                if(numBytes > 0){
                    connection_t * connP;
                    // check if endpoint is our server
                    connP = connection_find(dataP->connList, &addr, addrLen);
                    if (connP != NULL) {
                        
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

                        // MOVE MEMORY HERE
                        //
                        memcpy( &binFile[binFileCurrSize], response->payload, response->payload_len);
                        //
                        // MOVE MEMORY ABOVE
                        
                        //check first packet
                        if(block2_num == 0){
                            if(expBinFileSize < response->size){
                                expBinFileSize = response->size;
                            }
                        }
                        
                        binFileCurrSize += response->payload_len;
                        
                        lwm2m_free(response->payload);
                        response->payload = NULL;
                        response->payload_len = 0;
                        
                        xprintf("Downloaded %d%c \r\n", (100*binFileCurrSize)/expBinFileSize, 37);
                        
                        //check end condition // received last response
                        if(block2_more == 0){
                            xprintf("\r\nTotal binPackage size:%d, expected:%d\r\n", binFileCurrSize, expBinFileSize);
                            if(expBinFileSize !=  binFileCurrSize){
                                printErr("invalide binary size!", response);
                                return -1;
                            }
                            
                            // checksum
                            memset(&binFile[binFileCurrSize], 0 , 1);
                            
                            OTAMetadata_t * firmware_struct = (OTAMetadata_t*)&binFile[FIRMWARE_STRUCT_OFFSET];
                            uint32_t crc_result =  crc32(&binFile[FIRMWARE_STRUCT_SIZE], 
                                    binFileCurrSize-FIRMWARE_STRUCT_SIZE);
                            #ifdef COAPLOG
                            xprintf( "file CRC: %d, real CRC: %d \r\n", 
                                    crc_result, firmware_struct->crc);
                            #endif
                            if(crc_result != firmware_struct->crc){
                                printErr("CRC does not match", response);
                            }
                            fprintf(stdout, "\n\t DOWNLOAD COMPLETED\r\n");
                            break; // last packet finished;
                        }
                    } 
                    // packet from different source, reject
                    else {
                        /*
                         * This packet comes from an unknown peer
                         */
                        fprintf(stderr, "received bytes ignored!\r\n");
                    }
                }
            }
        }      
        retrys = 0;
    }
    return 0;  
}