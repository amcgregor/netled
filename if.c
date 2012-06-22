//
// File        : if.c
// Description : Monitor the throughput of network devices.
// Version     : 1.0
// Author      : Morgan Hughes (kyhm@kyhm.com)
// Maintainer  : Alice Bevan-McGregor (alice@gothcandy.com)
//
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#include "datatype.h"
#include "utils.h"
#include "if.h"

int get_if_throughput (if_throughput_t *ifp) {
  llong  ct = timer(),
         et,
	 x;
  char   buff[1024],
         srch[16],
        *p,
	*q;
  FILE  *fp;
  int    i;

  if (!ifp) {
    errno = EINVAL;
    return (-1); 
  }

  if ( (fp = fopen ("/proc/net/dev", "r")) ) {
    sprintf (srch, "%s:", ifp->if_name);
    while (fgets(buff, 1024, fp)) {
      if ( (q = strrchr(buff, '\n')) ) *q-- = '\0';
      for (p=buff; *p && isspace(*p); p++) ;
      if (strstr(p, srch) == p) {
        while (*p && *p != ':') p++;
        while (*p && *p == ':') p++;

        x = atoll(p);
        ifp->chg_bytes_in = x - ifp->tot_bytes_in;
        ifp->tot_bytes_in = x;
    
        for (i=0; i<9; i++) {
          while (*p && !isspace(*p)) p++;
          while (*p &&  isspace(*p)) p++;
        }
    
        x = atoll(p);
        ifp->chg_bytes_out = x - ifp->tot_bytes_out;
        ifp->tot_bytes_out = x;

        if (!ifp->last_ms) {
          ifp->last_ms = ct; 
          fclose (fp);
          errno = EAGAIN;
          return (-1); 
        } 

        et = (ct - ifp->last_ms);
        ifp->sec_bytes_in = (1000 * ifp->chg_bytes_in) / et;
        ifp->sec_bytes_out = (1000 * ifp->chg_bytes_out) / et; 
    
        fclose (fp);
        ifp->last_ms = ct; 
        ifp->time_ms = et; 
        return (0); 
      }

    }
    fclose (fp);
    errno = ENOENT;
    return (-1);
  }
  else
    return (-1); 
}

int is_interface_valid (char *interface) {
  FILE *fp = NULL;
  char line[256];
  
  fp = fopen ("/proc/net/dev", "r");

  while ( !feof (fp) ) {
    char *name;

    fgets(line, sizeof(line)/sizeof(char), fp);

    name = strchr(line, ':');
    if(name == NULL) 
      break;
    *name = '\0';
    
    /* trim leading spaces */
    for(name = line; *name == ' '; name++);
    
    if(strcmp(name,interface) == 0)
      return 1;
  }
  fclose(fp);
  
  return 0;
}

// This file can be run as a stand-alone program, and can also be compiled this
// way.  All you need to define is THROUGHPUT_MAIN, and it works.
// Easier yet is for you to type: "make iftest" on the command line :)

#ifdef THROUGHPUT_MAIN
int main (int argc, char **argv) {
  if_throughput_t  eth0 = { "eth0", 0, 0, 0, 0, 0, 0 }; 
  if (argv[1] && is_interface_valid(argv[1]))
    strncpy (eth0.if_name, argv[1], 16);

  printf ("Checking interface \"%s\"...\n"
          "%8s  %8s  %9s  %9s\n", 
	  eth0.if_name,
          "Bytes In", "By In /s", "Bytes Out", "By Out /s");
  while (1) {
    if (!get_if_throughput (&eth0))
      printf ("%8lld  %8lld  %9lld  %9lld\n", 
              eth0.chg_bytes_in,
              eth0.sec_bytes_in,
              eth0.chg_bytes_out,
              eth0.sec_bytes_out);
    usleep (250000);
  }

  return (0);
}
#endif