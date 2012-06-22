#include <stdio.h>
#include <linux/kd.h>
#include "configfile.h"
  
NetledConfig *getconf( NetledConfig *cfg, FILE *configfile ) {
  int ch = 0;
  char buf[LINEBUFSIZE];
  char ledname[LINEBUFSIZE];  
  char funcname[LINEBUFSIZE];  
  char *bufptr;
  char *bufmax;

  if( configfile == NULL || cfg == NULL ) {
    fprintf( stderr, "error: config file or cfg is null\n" );
    return( NULL );
  }
  
  bufmax = buf + LINEBUFSIZE;
  
  while( ch != EOF ) {
    ch = fgetc( configfile );
    bufptr = buf;
    /* get line into buf */
    while( ch != '#' && ch != '\n' && ch != EOF && bufptr < bufmax ) {
      *bufptr = ch;
      bufptr++;
      ch = fgetc( configfile );
    }
    *bufptr = '\0';

    if( sscanf( buf, "%s %s", &ledname, &funcname ) == 2 ) { /* parse line */
      printf( "got \"%s\"=\"%s\"\n", funcname, ledname );
      if( strcmp( SENDIDENT, funcname ) == 0 )
	cfg->sendled = leds( ledname );
      else if( strcmp( RECVIDENT, funcname ) == 0 )
	cfg->recvled = leds( ledname );
      else if( strcmp( CARRIDENT, funcname ) == 0 )
	cfg->carrled = leds( ledname );
      else
	fprintf( stderr, "warning: bad line in config, sipping...\n" );
    }
    
    while( ch != '\n' && ch != EOF ) /* dump to end of line */
      ch = fgetc( configfile );
  }

  return( cfg );
}
