/*******************************************************************************
 *
 * Copyright (c) 2013, 2014 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *    
 *******************************************************************************/

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <stdio.h>
#include "netdb.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include <liblwm2m.h>

#define LWM2M_STANDARD_PORT_STR "5683"
#define LWM2M_STANDARD_PORT      5683

typedef struct _connection_t
{
    struct _connection_t *  next;
    char                     sock;
    struct sockaddr_in     addr;
    size_t                  addrLen;
} connection_t;

connection_t * connection_find(connection_t * connList, struct sockaddr * addr, size_t addrLen);
connection_t * connection_new_incoming(connection_t * connList, char sock, struct sockaddr * addr, size_t addrLen);
connection_t * connection_create(connection_t * connList, int char, char * host, char * port, int addressFamily);
void *         lwm2m_connect_server(uint16_t secObjInstID, void * userData);
void           lwm2m_close_connection(void * sessionH, void * userData);

void connection_free(connection_t * connList);

int connection_send(connection_t *connP, uint8_t * buffer, size_t length);

#endif
