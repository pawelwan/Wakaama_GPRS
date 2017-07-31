#ifndef __STATE_MACHINE_H
#define __STATE_MACHINE_H

#include "FreeRTOS.h"

// initialize tcp, udp and rx queue
uint8_t queues_init();

// send 1 byte to queues
void send_to_queues(char chr);

// receive 1 byte from rx queue
BaseType_t receive_from_rx(char *key, TickType_t timeout);

// receive 1 byte from udp queue
BaseType_t receive_from_udp(char *key, TickType_t timeout);

// receive 1 byte from tcp queue
BaseType_t receive_from_tcp(char *key, TickType_t timeout);

#endif /* __STATE_MACHINE_H */
