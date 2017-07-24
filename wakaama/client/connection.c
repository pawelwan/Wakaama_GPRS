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
 *    Pascal Rieux - Please refer to git log
 *    
 *******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "connection.h"
#include "platform.h"

connection_t * connection_find(connection_t * connList,
                               const uint8_t * addr,
                               const uint8_t * port)
{
    connection_t * connP;

    connP = connList;
    while (connP != NULL)
    {
        if( (strcmp(connP->addr, addr) == 0)
            && (strcmp(connP->port, port) == 0)){
            return connP;
        }
        connP = connP->next;
    }

    return connP;
}

connection_t * connection_new_incoming(connection_t * connList,
                                        uint8_t sock,
                                        const uint8_t * addr,
                                        const uint8_t * port)
{
    connection_t * connP;

    connP = (connection_t *)lwm2m_malloc(sizeof(connection_t));
    if (connP != NULL)
    {
        connP->sock = sock;
        strcpy(&(connP->addr), addr);
        strcpy(&(connP->port), port);
        connP->next = connList;
    }

    return connP;
}

connection_t * connection_create(connection_t * connList, uint8_t sock,
                                                            char * host,
                                                            char * port){

    uint8_t s;
    connection_t * connP = NULL;
    
    connP = connection_new_incoming(connList, sock, host, port);

    return connP;
}

void connection_free(connection_t * connList){
    while (connList != NULL){
        connection_t * nextP;

        nextP = connList->next;
        lwm2m_free(connList);

        connList = nextP;
    }
}

int connection_send(connection_t *connP,
                    uint8_t * buffer,
                    size_t length){
    int nbSent;
    size_t offset;

#ifdef WITH_LOGS

    fprintf(stderr, "Sending %d bytes to [%s]:%s\r\n", length, connP->addr, connP->port);

    output_buffer(stderr, buffer, length, 0);
#endif

    offset = 0;
    while (offset != length)
    {
        nbSent = g510_sendPacket(connP->sock, buffer + offset, length - offset);
        if (nbSent < 0) return -1;
        offset += nbSent;
    }
    return 0;
}

uint8_t lwm2m_buffer_send(void * sessionH,
                          uint8_t * buffer,
                          size_t length,
                          void * userdata)
{
    connection_t * connP = (connection_t*) sessionH;

    if (connP == NULL)
    {
        fprintf(stderr, "#> failed sending %lu bytes, missing connection\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    if (-1 == connection_send(connP, buffer, length))
    {
        fprintf(stderr, "#> failed sending %lu bytes\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    return COAP_NO_ERROR;
}

bool lwm2m_session_is_equal(void * session1,
                            void * session2,
                            void * userData)
{
    return (session1 == session2);
}
