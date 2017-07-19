#include "liblwm2m.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/*
 *  Resources:
 *
 *          Name         | ID | Operations | Instances | Mandatory |  Type   |  Range  | Units |
 *  Test Identifier      |  0 |    R       |  Single   |    Yes    | String  |         |       |
 *  Start Time           |  1 |    R       |  Single   |    No     | String  |         |  Sec  |
 *  Duration In Seconds  |  2 |    R       |  Single   |    Yes    | String  |         |  Sec  |
 *  Test State           |  3 |    R       |  Single   |    Yes    | String  |         |       |
 *  Run                  |  4 |    E       |  Single   |    Yes    | String  |         |       |
 *  Progres              |  5 |    R       |  Single   |    No     | Integer |  0-100  |   %   |
 *  Result               |  6 |    R       |  Single   |    Yes    | Float   |         |       |
 *  Avg Test Result      |  7 |    R       |  Single   |    No     | Float   |         |       |
 *
 */

// Obj Id
#define LWM2M_TEST_OBJECT_ID   26241 // private pool

// Resource Id's:
#define RES_M_ID               0
#define RES_O_START_TIME       1
#define RES_M_DURATION         2
#define RES_M_STATE            3
#define RES_M_RUN              4
#define RES_O_PROGRES          5
#define RES_M_RESULT           6
#define RES_O_AVG_RESULT       7

#define PRV_TEST_NAME_LENGHT   10
#define PRV_TEST1              "testname1"
#define PRV_TEST2              "testname2"

#define PRV_RESULTS_MAX        10

// Amount Of Instances
#define PRV_TEST_INSTANCE_AMOUNT   2

typedef struct _test_instance_
{
    struct _test_instance_ * next;   // matches lwm2m_list_t::next
    uint16_t  instanceId;            // matches lwm2m_list_t::id
    time_t    start_time;
    time_t    duration_time;
    uint8_t   state;
    uint8_t   progres;
    float   result;
    char      test_name[PRV_TEST_NAME_LENGHT];
    float     results[PRV_RESULTS_MAX];
} test_instance_t;


static uint8_t prv_set_value(lwm2m_data_t * dataP,
                             test_instance_t * targetP)
{
    // a simple switch structure is used to respond at the specified resource asked
    switch (dataP->id)
    {
    case RES_M_ID:
        lwm2m_data_encode_string(targetP->test_name, dataP);
        return COAP_205_CONTENT;
        
    case RES_O_START_TIME:
        lwm2m_data_encode_int(targetP->start_time, dataP);
        return COAP_205_CONTENT;
        
    case RES_M_DURATION:
        lwm2m_data_encode_int(targetP->duration_time, dataP);
        return COAP_205_CONTENT;

    case RES_M_RUN:
        return COAP_405_METHOD_NOT_ALLOWED;
    
    case RES_M_STATE:
        lwm2m_data_encode_int(targetP->state, dataP);
        return COAP_205_CONTENT;    
        
    case RES_O_PROGRES:
        lwm2m_data_encode_int(targetP->progres, dataP);
        return COAP_205_CONTENT;    
        
    case RES_M_RESULT:
        lwm2m_data_encode_float(targetP->result, dataP);
        return COAP_205_CONTENT;    
    
    case RES_O_AVG_RESULT:{
        float result = 0.0;
        for(int i = 0; i < PRV_RESULTS_MAX; i++ ){
            result += targetP->results[i];
        }
        result /= PRV_RESULTS_MAX;
        lwm2m_data_encode_float(result, dataP);
        return COAP_205_CONTENT;    
    }
    default:
        return COAP_404_NOT_FOUND;
    }
}

static uint8_t prv_test_read(uint16_t instanceId,
                               int * numDataP,
                               lwm2m_data_t ** dataArrayP,
                               lwm2m_object_t * objectP)
{
    test_instance_t * targetP;
    uint8_t result;
    int i;

    targetP = (test_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP) return COAP_404_NOT_FOUND;

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {
                RES_M_ID,               
                RES_O_START_TIME,       
                RES_M_DURATION,         
                RES_M_STATE,      
                RES_O_PROGRES,          
                RES_M_RESULT,           
                RES_O_AVG_RESULT
                
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

static uint8_t prv_test_discover(uint16_t instanceId,
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
                RES_M_ID,               
                RES_O_START_TIME,       
                RES_M_DURATION,         
                RES_M_STATE,      
                RES_O_PROGRES,
                RES_M_RUN, 
                RES_M_RESULT,           
                RES_O_AVG_RESULT

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
                case RES_M_ID:
                case RES_O_START_TIME:       
                case RES_M_DURATION:       
                case RES_M_STATE:   
                case RES_O_PROGRES:
                case RES_M_RUN:
                case RES_M_RESULT:          
                case RES_O_AVG_RESULT:
                break;
            default:
                result = COAP_404_NOT_FOUND;
            }
        }
    }

    return result;
}

static uint8_t prv_test_write(uint16_t instanceId,
                                int numData,
                                lwm2m_data_t * dataArray,
                                lwm2m_object_t * objectP)
{
    
    int i;
    uint8_t result;
    test_instance_t * targetP;
    
    targetP = (test_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP)
    {
        return COAP_404_NOT_FOUND;
    }

    i = 0;
    do
    {
        switch (dataArray[i].id)
        {
            case RES_M_ID:
            case RES_O_START_TIME:       
            case RES_M_DURATION:       
            case RES_M_STATE:   
            case RES_O_PROGRES:
            case RES_M_RUN:
            case RES_M_RESULT:          
            case RES_O_AVG_RESULT:
                return COAP_405_METHOD_NOT_ALLOWED;

            default:
                return COAP_404_NOT_FOUND;
            }
        i++;
    } while (i < numData && result == COAP_204_CHANGED);

    return result;
}

static uint8_t prv_test_execute(uint16_t instanceId,
                                  uint16_t resourceId,
                                  uint8_t * buffer,
                                  int length,
                                  lwm2m_object_t * objectP)

{
    
    test_instance_t * targetP;
    
    targetP = (test_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP)
    {
        return COAP_404_NOT_FOUND;
    }
    switch (resourceId){
        case RES_M_ID:
        case RES_O_START_TIME:       
        case RES_M_DURATION:       
        case RES_M_STATE:   
        case RES_O_PROGRES:
        case RES_M_RESULT:          
        case RES_O_AVG_RESULT:
            return COAP_405_METHOD_NOT_ALLOWED;
        case RES_M_RUN:
            targetP->start_time = time(NULL);
            xprintf("started test %s \r\n",targetP->test_name );
            //start execution of tests remember about statuses progres etc
            targetP->duration_time = time(NULL) - targetP->start_time;
            return COAP_204_CHANGED;
            break;

        default:
            return COAP_404_NOT_FOUND;
        }
}

lwm2m_object_t * get_object_test()
{
    
    lwm2m_object_t * testObj;

    testObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));

    if (NULL != testObj)
    {
        test_instance_t * testInstance;
        
        memset(testObj, 0, sizeof(lwm2m_object_t));

        testObj->objID = LWM2M_TEST_OBJECT_ID;
        
        for(int i = 1; i<= PRV_TEST_INSTANCE_AMOUNT; i++){
            testInstance = (test_instance_t *)lwm2m_malloc(sizeof(test_instance_t));
            if (NULL == testInstance)
            {
                lwm2m_free(testObj);
                return NULL;
            }

            memset(testInstance, 0, sizeof(test_instance_t));
            testInstance->instanceId = i;
            switch(testInstance->instanceId){
                case 1:
                    strcpy(testInstance->test_name, PRV_TEST1);
                    break;
                case 2:
                    strcpy(testInstance->test_name, PRV_TEST2);
                    break;
            }
            for(int j = 0; j < PRV_RESULTS_MAX; j++){
                testInstance->results[j] = 0.0;
            }
            testInstance->state = 0;
            
            testObj->instanceList = LWM2M_LIST_ADD(testObj->instanceList, testInstance);
        }
        
        /*
         * And the private function that will access the object.
         * Those function will be called when a read/write/execute query is made by the server. In fact the library don't need to
         * know the resources of the object, only the server does.
         */
        testObj->readFunc     = prv_test_read;
        testObj->writeFunc    = prv_test_write;
        testObj->discoverFunc = prv_test_discover;
        testObj->executeFunc  = prv_test_execute;

     }

    return testObj;
}

void free_object_test(lwm2m_object_t * objectP)
{
    if (NULL != objectP->userData)
    {
        lwm2m_free(objectP->userData);
        objectP->userData = NULL;
    }
    while (objectP->instanceList != NULL)
    {
        test_instance_t * testInstance = (test_instance_t *)objectP->instanceList;
        objectP->instanceList = objectP->instanceList->next;
        lwm2m_free(testInstance);
    }
    objectP->instanceList = NULL;

    lwm2m_free(objectP);
}
