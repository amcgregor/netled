//
// File        : ledctrl.h
// Description : Monitor the throughput of network devices.
// Version     : 1.0
// Author      : Alice Bevan-McGregor (alice@gothcandy.com)
// Maintainer  : Alice Bevan-McGregor (alice@gothcandy.com)
//

#ifndef HAVE_LEDCTRL_H
#define HAVE_LEDCTRL_H

#define LEDOFF  0
#define LEDON   1

void toggleled ( int ttyfd, unsigned char led, unsigned char status );
void show_interfaces ( void );

#endif