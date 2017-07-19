#include <stdio.h>
#include "common.h"
#include "platformtime.h"

time_t time_ (time_t * timer){
    time_t time = (time_t)(xTaskGetTickCount()/TICSPERSECOND);
    if (timer != NULL){
        *timer = time;
    }
    return time;
}

int gettimeofday_( struct timeval *tv, void *tzvp )
{
    time_t t;
    time(&t);  // 1
    tv->tv_sec = t;  // convert to seconds
    tv->tv_usec = 0;  // get remaining microseconds
    return 0;  // return non-zero for error
}

