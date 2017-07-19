/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#ifndef LWM2MCLIENT_H_
#define LWM2MCLIENT_H_

#include "wakaama/liblwm2m.h"
#include "connection.h"
#include "config_client.h"
#include "firmware/firmware_update.h"
#include "wakaama/platform/platform.h"
#include "wakaama/platform/platformtime.h"


lwm2m_object_t * get_object_device(void);
lwm2m_object_t * get_object_server(void);
lwm2m_object_t * get_object_security(void);
lwm2m_object_t * get_object_led(void);
lwm2m_object_t * get_object_accelerometer(void);
lwm2m_object_t * get_object_firmware(lwm2m_context_t ** context);
lwm2m_object_t * get_object_test(void);

void free_object_device(lwm2m_object_t * objectP);
void free_object_security(lwm2m_object_t * object);
void free_object_server(lwm2m_object_t * objectP);
void free_object_led(lwm2m_object_t * objectP);
void free_object_accelerometer(lwm2m_object_t * objectP);
void free_object_firmware(lwm2m_object_t * objectP);
void free_object_test(lwm2m_object_t * objectP);
char * get_server_uri(lwm2m_object_t * objectP, uint16_t secObjInstID);
#endif