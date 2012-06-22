/*
 * File    : utils.c
 * Descript: 
 * Version :
 * Author  : Morgan Hughes (kyhm@kyhm.com)
 */

#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <netdb.h>

#include "datatype.h"

/* Return the system time in milliseconds.  This is returned in a long long
 * under GCC, or by a double otherwise. */
llong timer()
{
  static struct timeval  tv;
  register llong t;
  gettimeofday(&tv, NULL);
  t  = tv.tv_sec;
  t *= 1000;
  t += (tv.tv_usec / 1000);
  return ( t );
}

