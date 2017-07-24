#ifndef __G510_SOCKET_H
#define __G510_SOCKET_H

#define MAX_PACKET_SIZE 1024

char g510_openSocket(void);
char g510_closeSocket(char socket);

uint32_t g510_sendPacket(char socket, const char* s, size_t length);
uint32_t g510_udpRead(char* packet, char socket, const char * adress, const char* port);

#endif /* __G510_SOCKET_H */
