//
// File        : daemon.c
// Description : Monitor the throughput of network devices.
// Version     : 1.0
// Author      : Alice Bevan-McGregor (alice@gothcandy.com)
// Maintainer  : Alice Bevan-McGregor (alice@gothcandy.com)
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

// get pid from /var/run/netled.pid
int get_pid ( void ) {
  int pid;
  FILE *PID = NULL;
  char line[256];

  PID = fopen ( "/var/run/netled.pid", "r" );
  if ( NULL == PID || feof ( PID ) )
    return -1;

  while ( !feof ( PID ) ) {
    fgets ( line, 255, PID );
    pid = atoi ( line );
    if ( pid > 1 ) {
      fclose ( PID );
      return pid;
    }
  }
  
  fclose ( PID );
  return -1;
}

// write pid to /var/run/netled.pid
void put_pid ( pid_t pid ) {
  FILE *PID = NULL;
  PID = fopen ( "/var/run/netled.pid", "w" );
  if ( PID == NULL )
    fprintf ( stderr, "Coulndt open pid file for writing\n" );
  fprintf ( PID, "%i\n", pid );
  fclose ( PID );
}

// comment goes here, but you should know what this does
void daemon_kill ( int sig ) {
  unlink ( "/var/run/netled.pid" );
  exit ( 0 );
}

// fork a child, and exit parent if succesfull
void daemonize ( void ) {
  pid_t me;				    // pid of process
  int oldpid;                               // old pid of proces
  
  // check if allready running, if so dont start another
  oldpid = get_pid();
  if ( oldpid !=- 1 ) {
    fprintf ( stderr, "Error: NetLED allready running with pid %i.\n", oldpid);
    exit ( 1 );
  }
  
  // install signal handler before we fork
  signal(SIGTERM,&daemon_kill);
  
  // we start off by forking ourself
  me = fork();
  if ( 0 != me ) {
    // in the parent did things go ok
    if ( -1 != me ) {
      // things went ok, write out pid and exit
      put_pid ( me );
      exit ( 0 );
    } else {
      // there was an error
      fprintf(stderr, "Error: problem forking.");
      exit ( 1 );
    }
  }
}