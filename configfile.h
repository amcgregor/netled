
#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "netleds.h"

typedef struct {
  Led sendled;
  Led recvled;
  Led carrled;
} NetledConfig;

#define SENDIDENT "SD"
#define RECVIDENT "RD"
#define CARRIDENT "CD"

NetledConfig *getconf( NetledConfig *cfg, FILE *configfile );

#define LINEBUFSIZE 1024

#endif /* CONFIGFILE_H */
