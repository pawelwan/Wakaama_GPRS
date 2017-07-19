#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "g510_frt.h"

#include "wakaama/client/lwm2m_client.h"

static uint8_t g510_get_free_socket(void){
    
    if(g510WaitForRegistration(300)) return -1;
    
    g510FlushRx();
    g510TxCmd("AT+MIPOPEN?\r");
    
    char buf[1024];
    g510ReadResp(buf,1024,30000);
    char key[] = "1234567890"; // anyway only 1,2,3,4 are valid, 0 gives error
    char * pch = strpbrk (buf, key);
    
    return *pch;
}

uint8_t g510_close_socket(char socket){
    
    if(g510WaitForRegistration(300)) return -1;
    
    g510FlushRx();
    g510TxCmd("AT+MIPCLOSE=\" "socket" \"\r");
    
    if(g510WaitForResponse("OK",30000)) return 0;
    
    return -1;
}

static int g510_bind_server_socket(char socket){ // protocol -> 0 = TCP, 1 = UDP
    
    if(g510WaitForRegistration(300)) return -1;
    
    g510FlushRx();
    g510TxCmd("AT+MIPOPEN=\" "socket" \",\" "LOCALPORT" \",\" "SERVER_IP" \",\" "SERVER_PORT" \",1\r");
    
    if(g510WaitForResponse("OK",30000)) return 0;
    
    return -1;
}

uint8_t g510_open_socket(void){
    uint8_t socket = g510_get_free_socket(void);
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

uint32_t g510_udp_read(const char * buff){
}