//
// File        : daemon.h
// Description : Monitor the throughput of network devices.
// Version     : 1.0
// Author      : Alice Bevan-McGregor (alice@gothcandy.com)
// Maintainer  : Alice Bevan-McGregor (alice@gothcandy.com)
//

#ifndef HAVE_DAEMON_H
#define HAVE_DAEMON_H

int daemonized = 0;

int get_pid ( void );
void put_pid ( pid_t pid );
void daemon_kill ( int sig );
void daemonize ( void );

#endif