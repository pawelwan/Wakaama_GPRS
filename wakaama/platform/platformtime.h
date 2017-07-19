#ifndef _PLATFROM_TIME_H_
#define _PLATFORM_TIME_H_

#include <stdlib.h>
#include "lwip/sockets.h" // for timeval

/* In our STM we dont have real time clock so we need to use own versions of 
 * time functions, this includes time() and gettimeofday(),
 * to use our functions set TIMEINTERCEPT for 1, for default time.h funs set 0
 */
#ifndef TIMEINTERCEPT
#define TIMEINTERCEPT 1
#endif

/*  look at sw_eth\lwip\src\include\lwip\sockets.h line 306, 
 * we need also time_t which we put here
 */
#ifndef _SYS__TIMEVAL_H_ 
    #define time_t long
    typedef long time_t;
#endif

#if TIMEINTERCEPT
    #define time(x) time_(x)
    #define gettimeofday(x, y) gettimeofday_(x, y)
    #define _gettimeofday(x, y) gettimeofday_(x, y) // fun added in arm gcc 5.1
#endif

#define TICSPERSECOND ( pdMS_TO_TICKS(1000) )

time_t time_(time_t * timer);
int gettimeofday_(struct timeval *tv, void *tzvp);

#endif /* _PLATFROM_TIME_H_ */