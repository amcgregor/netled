//
// File        : config.h
// Description : The configuration header file.
// Version     : 4.0
// Author      : Alice Bevan-McGregor (alice@gothcandy.com)
// Maintainer  : Alice Bevan-McGregor (alice@gothcandy.com)
//

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "config.h"

typedef struct {
  int sendled;
  int recvled;
  int carrled;
} NetledConfig;

#define SENDIDENT "SD"
#define RECVIDENT "RD"
#define CARRIDENT "CD"

#define LINEBUFSIZE 1024

NetledConfig *getconf( NetledConfig *cfg, FILE *configfile );

#endif
