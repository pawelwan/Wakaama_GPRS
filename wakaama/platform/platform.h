#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <stdlib.h>


void * lwm2m_malloc(size_t s);
void lwm2m_free(void * p);
char * lwm2m_strdup(const char * str);
int lwm2m_strncmp(const char * s1, const char * s2, size_t n);
time_t lwm2m_gettime(void);
void lwm2m_printf(const char * format, ...);


#endif /* PLATFORM_H */