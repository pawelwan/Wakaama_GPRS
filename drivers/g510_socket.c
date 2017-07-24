#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "g510_frt.h"
#include "g510_socket.h"

#include "wakaama/client/lwm2m_client.h"

static char g510_get_free_socket(void){
    
    if(g510WaitForRegistration(G510_REGISTER_TIMEOUT)) return -1;
    
    g510FlushRx();
    g510TxCmd("AT+MIPOPEN?\r");
    
    char buf[1024];
    g510ReadResp(buf,1024,G510_RESPONSE_TIMEOUT);
    char key[] = "1234567890"; // anyway only 1,2,3,4 are valid, 0 gives error
    char * pch = strpbrk (buf, key);
    
    return *pch;
}

char g510_closeSocket(char socket){
    
    char cmdBuf[20];
    sprintf(cmdBuf,"AT+MIPCLOSE=%c\r",socket);
    
    if(g510WaitForRegistration(G510_REGISTER_TIMEOUT)) return -1;
    
    g510FlushRx();
    g510TxCmd(cmdBuf);
    
    if(g510WaitForResponse("OK",G510_RESPONSE_TIMEOUT)) return 0;
    
    return -1;
}

static int g510_bind_server_socket(char socket){ // protocol -> 0 = TCP, 1 = UDP
    
    char cmdBuf[100];
    sprintf(cmdBuf,"AT+MIPOPEN=%c,%s,%s,%s,1\r",socket, LOCAL_PORT_STR, SERVER_IP_STR, SERVER_PORT_STR);
    
    if(g510WaitForRegistration(G510_REGISTER_TIMEOUT)) return -1;
    
    g510FlushRx();
    g510TxCmd(cmdBuf);
    
    if(g510WaitForResponse("OK",G510_RESPONSE_TIMEOUT)) return 0;
    
    return -1;
}

char g510_openSocket(void){
    char socket = g510_get_free_socket();
    if(socket > '4' || socket < '1'){
        xprintf("g510_get_free_socket error %u", socket);
        return '5';
    }
    if(g510_bind_server_socket(socket)){
        xprintf("g510_bind_server_socket error");
        return '6';
    }
    return socket;
}

uint32_t g510_sendPacket(char socket, const char* s, size_t length){
    xprintf("%c send: %s\n", socket, s);

    char cmdBuf[20];

    sprintf(cmdBuf,"AT+MIPSEND=%c,\r",socket);

    int len = strlen(cmdBuf);

    usart_txBuf(G510_USART,(uint8_t*)cmdBuf,len);

    //xprintf("tcpTx: len=%d\n",len);
    
    if(length > MAX_PACKET_SIZE){
        length = MAX_PACKET_SIZE;
    }

    //the data to tx:
    txBinData((const uint8_t*)s,length);

    strcpy(cmdBuf,"\"\r\n");
    usart_txBuf(G510_USART,(uint8_t*)cmdBuf,3);

    if(g510WaitForResponse("+MIPSEND",G510_RESPONSE_TIMEOUT)) return -1;
     // by default timeout is 0 so it should work anyway
    sprintf(cmdBuf,"AT+MIPPUSH=%c\r",socket);
    
    g510FlushRx();
    g510TxCmd(cmdBuf);
    if(g510WaitForResponse("OK",G510_RESPONSE_TIMEOUT)) return -2;
    
    return length;
}
/* // is this working just for sending packets? if for received too then its so usefull
uint32_t g510_read(const char* packet, char socket){
    
    char buff[50];
    
    g510FlushRx();
    g510TxCmd("AT+MIPREAD?");
    
    if(g510ReadResp(buff, sizeof(buff), G510_RESPONSE_TIMEOUT)){
        sscanf(buff ,"+MIPREAD:%c，%d", )
    }    
    
}
*/

static uint32_t hexStringToBin(char * buff, char * string, size_t size){
    
    size_t count;
    size_t len = size = size/2 + size%2;
    
    if(size % 2 == 1){
        string[size] = 0; // will need 1 more argument at the end, it cannot be null
    }
    
    for(count = 0; count < len; count++, string +=2) {
        sscanf(string, "%2hhx", &buff[count]);
    }
    
    if(size % 2 == 1){
        buff[count] >>= 4;
        buff[count] &= 0x0F;  // we want 4 zeros at the beginning
    }
    
    return count;
}

uint32_t g510_udpRead(char* packet, char socket, const char * adress, const char* port){
    
    char buff[PACKET_SIZE * 2];
    char recv_addr[16];
    char * offset;
    char * step;
    uint32_t len;
    
    if(g510ReadResp(buff, sizeof(buff), G510_RESPONSE_TIMEOUT)){
        step = strstr(buff, "+MIPRUDP:");
        if(step == NULL){  // no message
            return 0;
        }
        step += sizeof("+MIPRUDP:");
        offset = strpbrk(step, ",");
        len = offset - step;
        strncpy(recv_addr, step, len);
        recv_addr[len] = 0;
        if(strncmp(recv_addr, adress, len)){ // different ip than server
            return -1;
        }
        
        offset++;
        step = offset;
        offset = strpbrk(step, ",");
        len = offset - step;
        strncpy(recv_addr, step, len);
        recv_addr[len] = 0;
        if(strncmp(recv_addr, port, len)){ // different port than server
            return -2;
        }
        
        offset++;
        if(*offset == socket){ // different socket than server
            return -3;
        }
        
        offset += 2; // left in stack
        step = offset;
        offset = strpbrk(step, ",");
        if(*step != '0'){ // more data in protocol stack -> split of pockets?
            return -4;
        }
        
        offset++;
        step = offset; // received data 
        offset = strpbrk(step, "\r");
        len = offset - step;
        len = hexStringToBin(packet, step, len);
        
        return len;
        
    }

    return -5;
}