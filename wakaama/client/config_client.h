
#ifndef CONFIGS_H
#define CONFIGS_H

#include "connection.h"

#define STR(s) #s
#define PACKET_SIZE 256
#define LOCAL_PORT 5683
#define LOCAL_PORT_STR STR(5683)
#define SERVER_IP_STR STR(192.168.0.50)
#define SERVER_PORT 5683
#define SERVER_PORT_STR STR(5683)
#define SELECT_TIMEOUT 500 // miliseconds
#define NOTIFY_TIMEOUT 10000 // miliseconds
#define DEVICE_NAME "Engineer Project Device"
#define OBJ_COUNT 5


typedef struct{
    lwm2m_object_t * securityObjP;
    uint8_t sock;
    connection_t * connList;
    const uint8_t * adress;
    const uint8_t * port;
} client_data_t;

#endif /* CONFIGS_H */

