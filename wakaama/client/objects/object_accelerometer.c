#include "liblwm2m.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <lis3dh.h>
#include <def.h>

/*
 *  Resources:
 *
 *          Name         |  ID   | Operations | Instances | Mandatory |  Type   |         Units              |
 *  Min Range Value      |  5603 |     R      |  Single   |    No     |  Float  | Defined by “Units” resource.|
 *  Max Range Value      |  5604 |     R      |  Single   |    No     |  Float  | Defined by “Units” resource.|
 *  Sensor Units         |  5701 |     R      |  Single   |    No     | String  |                            |
 *  X Value              |  5702 |     R      |  Single   |    Yes    |  Float  | Defined by “Units” resource.|
 *  Y Value              |  5703 |     R      |  Single   |    No     |  Float  | Defined by “Units” resource.|
 *  Z Value              |  5704 |     R      |  Single   |    No     |  Float  | Defined by “Units” resource.|
 *
 */

// Obj Id
#define LWM2M_ACCELEROMETER_OBJECT_ID   3313

// Resource Id's:
#define RES_O_MIN_RANGE       5603
#define RES_O_MAX_RANCE       5604
#define RES_O_UNITS           5701
#define RES_M_X               5702
#define RES_O_Y               5703
#define RES_O_Z               5704

#define PRV_MIN_RANGE_VALUE              -100.0f
#define PRV_MAX_RANGE_VALUE              100.0f
#define PRV_ACCELEROMETER_SENSOR_UNITS   "g"

static float MAX_RANGE_VALUE =  0.0f;
static float MIN_RANGE_VALUE = 0.0f;

static uint8_t prv_set_value(lwm2m_data_t * dataP)
{
    // a simple switch structure is used to respond at the specified resource asked
    switch (dataP->id)
    {
    case RES_O_MIN_RANGE:
        lwm2m_data_encode_float(MIN_RANGE_VALUE, dataP);
        return COAP_205_CONTENT;
        
    case RES_O_MAX_RANCE:
        lwm2m_data_encode_float(MAX_RANGE_VALUE, dataP);
        return COAP_205_CONTENT;
        
    case RES_O_UNITS:
        lwm2m_data_encode_string(PRV_ACCELEROMETER_SENSOR_UNITS, dataP);
        return COAP_205_CONTENT;
        
    case RES_M_X:{
        float x = lis3dh_get_axis(LIS3DH_X_AXIS);
        MAX_RANGE_VALUE = LWIP_MAX(MAX_RANGE_VALUE, x);
        MIN_RANGE_VALUE = LWIP_MIN(MIN_RANGE_VALUE, x);
        lwm2m_data_encode_float(x, dataP);
        return COAP_205_CONTENT;
    }
        
    case RES_O_Y:{
        float y = lis3dh_get_axis(LIS3DH_Y_AXIS);
        MAX_RANGE_VALUE = LWIP_MAX(MAX_RANGE_VALUE, y);
        MIN_RANGE_VALUE = LWIP_MIN(MIN_RANGE_VALUE, y);
        lwm2m_data_encode_float(y, dataP);
        return COAP_205_CONTENT;
    }
        
    case RES_O_Z:{
        float z = lis3dh_get_axis(LIS3DH_Z_AXIS);
        MAX_RANGE_VALUE = LWIP_MAX(MAX_RANGE_VALUE, z);
        MIN_RANGE_VALUE = LWIP_MIN(MIN_RANGE_VALUE, z);
        lwm2m_data_encode_float(z, dataP);
        return COAP_205_CONTENT;
    }

    default:
        return COAP_404_NOT_FOUND;
    }
}

static uint8_t prv_accel_read(uint16_t instanceId,
                               int * numDataP,
                               lwm2m_data_t ** dataArrayP,
                               lwm2m_object_t * objectP)
{
    uint8_t result;
    int i;

    // this is a single instance object
    if (instanceId != 0)
    {
        return COAP_404_NOT_FOUND;
    }

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {
                RES_O_UNITS,
                RES_M_X,
                RES_O_Y,
                RES_O_Z,
                RES_O_MIN_RANGE,
                RES_O_MAX_RANCE
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
        result = prv_set_value((*dataArrayP) + i);
        i++;
    } while (i < *numDataP && result == COAP_205_CONTENT);

    return result;
}

static uint8_t prv_accel_discover(uint16_t instanceId,
                               int * numDataP,
                               lwm2m_data_t ** dataArrayP,
                               lwm2m_object_t * objectP)
{
    uint8_t result;
    int i;

    // this is a single instance object
    if (instanceId != 0)
    {
        return COAP_404_NOT_FOUND;
    }

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {
                RES_O_UNITS,
                RES_M_X,
                RES_O_Y,
                RES_O_Z,
                RES_O_MIN_RANGE,
                RES_O_MAX_RANCE
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
    else{
        for (i = 0; i < *numDataP && result == COAP_205_CONTENT; i++){
            switch ((*dataArrayP)[i].id)
            {
            case RES_O_UNITS:
            case RES_M_X:
            case RES_O_Y:
            case RES_O_Z:
            case RES_O_MIN_RANGE:
            case RES_O_MAX_RANCE:
                break;
            default:
                result = COAP_404_NOT_FOUND;
            }
        }
    }
    
    return result;
}

lwm2m_object_t * get_object_accelerometer()
{
    
    lwm2m_object_t * accelObj;

    accelObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));

    if (NULL != accelObj)
    {
        
        memset(accelObj, 0, sizeof(lwm2m_object_t));

        accelObj->objID = LWM2M_ACCELEROMETER_OBJECT_ID;
        
        accelObj->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
        if (NULL == accelObj->instanceList)
        {
            lwm2m_free(accelObj);
            return NULL;
        }

        memset(accelObj->instanceList, 0, sizeof(lwm2m_list_t));
                  
        /*
         * And the private function that will access the object.
         * Those function will be called when a read/write/execute query is made by the server. In fact the library don't need to
         * know the resources of the object, only the server does.
         */
        accelObj->readFunc     = prv_accel_read;
        accelObj->discoverFunc = prv_accel_discover;
       
     }

    return accelObj;
}

void free_object_accelerometer(lwm2m_object_t * objectP)
{
    if (NULL != objectP->userData)
    {
        lwm2m_free(objectP->userData);
        objectP->userData = NULL;
    }
    if (NULL != objectP->instanceList)
    {
        lwm2m_free(objectP->instanceList);
        objectP->instanceList = NULL;
    }

    lwm2m_free(objectP);
}