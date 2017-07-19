/*******************************************************************************
 *
 * Copyright (c) 2013, 2014, 2015 Intel Corporation and others.
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
 *******************************************************************************/

#include <liblwm2m.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "platform.h"
#include "platformtime.h"

#ifndef LWM2M_MEMORY_TRACE

void * lwm2m_malloc(size_t s)
{
    return mem_malloc(s);
}

void lwm2m_free(void * p)
{
    return mem_free(p);
}

char * lwm2m_strdup(const char * str)
{
    //return strdup(str); // doesnt work
    char* np = (char*)mem_malloc(strlen(str)+1);
    return np ? strcpy(np, str) : np;
}

#endif

int lwm2m_strncmp(const char * s1,
                     const char * s2,
                     size_t n)
{
    return strncmp(s1, s2, n);
}

time_t lwm2m_gettime(void)
{
    struct timeval tv;

    if (0 != gettimeofday(&tv, NULL))
    {
        return -1;
    }

    return tv.tv_sec;
}

void lwm2m_printf(const char * str, ...){
    	va_list arp;
	int d, r, w, s, l;


	va_start(arp, str);

	while ((d = *str++) != 0) {
		if (d != '%') {
			xputc(d); continue;
		}
		d = *str++; w = r = s = l = 0;
		if (d == '0') {
			d = *str++; s = 1;
		}
		while ((d >= '0')&&(d <= '9')) {
			w += w * 10 + (d - '0');
			d = *str++;
		}
		if (s) w = -w;
		if (d == 'l') {
			l = 1;
			d = *str++;
		}
		if (!d) break;
		if (d == 's') {
			xputs(va_arg(arp, char*));
			continue;
		}
		if (d == 'c') {
			xputc((char)va_arg(arp, int));
			continue;
		}
		if (d == 'u') r = 10;
		if (d == 'd') r = -10;
		if (d == 'X' || d == 'x') r = 16; // 'x' added by mthomas in increase compatibility
		if (d == 'b') r = 2;
		if (!r) break;
		if (l) {
			xitoa((long)va_arg(arp, long), r, w);
		} else {
			if (r > 0)
				xitoa((unsigned long)va_arg(arp, int), r, w);
			else
				xitoa((long)va_arg(arp, int), r, w);
		}
	}

	va_end(arp);
}
