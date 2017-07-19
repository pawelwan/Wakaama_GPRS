
#ifndef CONFIGS_H
#define CONFIGS_H

#define PACKET_SIZE 256
#define LOCALPORT "5683"
#define SERVER_IP "192.168.0.50"
#define SERVER_PORT "5683"
#define SELECT_TIMEOUT 5 // seconds
#define NOTIFY_TIMEOUT 10000 // miliseconds
#define DEVICE_NAME "Engineer Project Device"
#define OBJ_COUNT 7

typedef struct{
    lwm2m_object_t * securityObjP;
    int sock;
    connection_t * connList;
    int addressFamily;
} client_data_t;

#endif /* CONFIGS_H */

