#include "liblwm2m.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <leds.h>

/*
 *  Resources:
 *
 *          Name            |  ID   | Operations | Instances | Mandatory |  Type   | Range | Units |
 *  Sensor Units            |  5701 |    R       |  Single   |    No     | String  |       |       |
 *  Colour                  |  5706 |    R/W     |  Single   |    No     | String  |       |       |
 *  Cumulative active power |  5805 |    R       |  Single   |    No     | Float   |       |   Wh  |
 *  Power factor            |  5820 |    R       |  Single   |    No     | Float   |       |       |
 *  On/Off                  |  5850 |    R/W     |  Single   |    Yes    | Boolean |       |       |
 *  Dimmer                  |  5851 |    R/W     |  Single   |    No     | Integer | 1-100 |   %   |
 *  On Time                 |  5852 |    R/W     |  Single   |    No     | Integer |       |  Sec  |
 *
 */

// Obj Id
#define LWM2M_LIGHT_OBJECT_ID   3311

// Resource Id's:
#define RES_O_COLOUR          5706
#define RES_M_ON_OFF          5850

#define PRV_LED1_COLOR          "x0000FF"
#define PRV_LED2_COLOR          "FFA500"
#define PRV_LED3_COLOR          "xFF0000"

// Amount Of Instances
#define PRV_LED_INSTANCE_AMOUNT   3

#define color_size 8

typedef struct _led_instance_
{
    struct _led_instance_ * next;   // matches lwm2m_list_t::next
    uint16_t    instanceId;            // matches lwm2m_list_t::id
    char     color[color_size];
} led_instance_t;


static uint8_t prv_set_value(lwm2m_data_t * dataP,
                             led_instance_t * targetP)
{
    // a simple switch structure is used to respond at the specified resource asked
    switch (dataP->id)
    {
    case RES_O_COLOUR:
        lwm2m_data_encode_string(targetP->color, dataP);
        return COAP_205_CONTENT;
        
    case RES_M_ON_OFF:
        lwm2m_data_encode_bool(ledGetState(targetP->instanceId), dataP);
        return COAP_205_CONTENT;
    default:
        return COAP_404_NOT_FOUND;
    }
}

static uint8_t prv_led_read(uint16_t instanceId,
                               int * numDataP,
                               lwm2m_data_t ** dataArrayP,
                               lwm2m_object_t * objectP)
{
    led_instance_t * targetP;
    uint8_t result;
    int i;

    // this is a single instance object
    targetP = (led_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP) return COAP_404_NOT_FOUND;

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {
                RES_O_COLOUR,
                RES_M_ON_OFF
                
        };
        int nbRes = sizeof(resList)/sizeof(uint16_t);

        *dataArrayP = lwm2m_data_new(nbRes);
        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = nbRes;
        for (i = 0 ; i < nbRes ; i++)
        {
            (*dataArrayP)[i].id = resList[i];
        }
    }

    i = 0;
    do
    {
        result = prv_set_value((*dataArrayP) + i, targetP);
        i++;
    } while (i < *numDataP && result == COAP_205_CONTENT);

    return result;
}

static uint8_t prv_led_discover(uint16_t instanceId,
                               int * numDataP,
                               lwm2m_data_t ** dataArrayP,
                               lwm2m_object_t * objectP)
{
    int i;
    uint8_t result;
    
    result = COAP_205_CONTENT;


    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {
                RES_O_COLOUR,
                RES_M_ON_OFF
                
        };
        int nbRes = sizeof(resList)/sizeof(uint16_t);

        *dataArrayP = lwm2m_data_new(nbRes);
        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = nbRes;
        for (i = 0 ; i < nbRes ; i++)
        {
            (*dataArrayP)[i].id = resList[i];
        }
    }
    else
    {
        for (i = 0; i < *numDataP && result == COAP_205_CONTENT; i++)
        {
            switch ((*dataArrayP)[i].id)
            {
            case RES_O_COLOUR:
            case RES_M_ON_OFF:
                break;
            default:
                result = COAP_404_NOT_FOUND;
            }
        }
    }

    return result;
}

static uint8_t prv_led_write(uint16_t instanceId,
                                int numData,
                                lwm2m_data_t * dataArray,
                                lwm2m_object_t * objectP)
{
    
    int i;
    uint8_t result;
    led_instance_t * targetP;
    
    targetP = (led_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP)
    {
        return COAP_404_NOT_FOUND;
    }

    i = 0;
    do
    {
        switch (dataArray[i].id)
        {
            case RES_O_COLOUR:
            case RES_M_ON_OFF:
            {
                bool on;

                if (1 == lwm2m_data_decode_bool(dataArray + i, &on)) {

                    if (on) {
                        led(targetP->instanceId, LED_ON);

                    } else {
                        led(targetP->instanceId, LED_OFF);
                    }
                    result = COAP_204_CHANGED;
                } else {
                    result = COAP_400_BAD_REQUEST;
                }
                break;
            }

            default:
                return COAP_404_NOT_FOUND;
            }
        i++;
    } while (i < numData && result == COAP_204_CHANGED);

    return result;
} 

lwm2m_object_t * get_object_led()
{
    
    lwm2m_object_t * ledObj;

    ledObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));

    if (NULL != ledObj)
    {
        led_instance_t * ledInstance;
        
        memset(ledObj, 0, sizeof(lwm2m_object_t));

        ledObj->objID = LWM2M_LIGHT_OBJECT_ID;
        
        for(int i = 1; i<= PRV_LED_INSTANCE_AMOUNT; i++){
            ledInstance = (led_instance_t *)lwm2m_malloc(sizeof(led_instance_t));
            if (NULL == ledInstance)
            {
                lwm2m_free(ledObj);
                return NULL;
            }

            memset(ledInstance, 0, sizeof(led_instance_t));
            ledInstance->instanceId = i;
            switch(ledInstance->instanceId){
                case LED1:
                    strcpy(ledInstance->color, PRV_LED1_COLOR);
                    break;
                case LED2:
                    strcpy(ledInstance->color, PRV_LED2_COLOR);
                    break;
                case LED3:
                    strcpy(ledInstance->color, PRV_LED3_COLOR);
                    break;
            }
            ledObj->instanceList = LWM2M_LIST_ADD(ledObj->instanceList, ledInstance);
        }
        
        /*
         * And the private function that will access the object.
         * Those function will be called when a read/write/execute query is made by the server. In fact the library don't need to
         * know the resources of the object, only the server does.
         */
        ledObj->readFunc     = prv_led_read;
        ledObj->writeFunc     = prv_led_write;
        ledObj->discoverFunc = prv_led_discover;   
        
        led1(LED_OFF);
        led2(LED_OFF);
        led3(LED_OFF);

     }

    return ledObj;
}

void free_object_led(lwm2m_object_t * objectP)
{
    if (NULL != objectP->userData)
    {
        lwm2m_free(objectP->userData);
        objectP->userData = NULL;
    }
    while (objectP->instanceList != NULL)
    {
        led_instance_t * ledInstance = (led_instance_t *)objectP->instanceList;
        objectP->instanceList = objectP->instanceList->next;
        lwm2m_free(ledInstance);
    }
    objectP->instanceList = NULL;

    lwm2m_free(objectP);
}
