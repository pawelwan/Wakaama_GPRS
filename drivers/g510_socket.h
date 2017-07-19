#ifndef __G510_SOCKET_H
#define __G510_SOCKET_H

uint8_t g510_open_socket(void);
uint8_t g510_close_socket(char socket);

uint32_t g510_udp_read(const char * buff);

#endif /* __G510_SOCKET_H */
