//
// File        : ledctrl.c
// Description : Monitor the throughput of network devices.
// Version     : 1.0
// Author      : Alice Bevan-McGregor (alice@gothcandy.com)
// Maintainer  : Alice Bevan-McGregor (alice@gothcandy.com)
//

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

#include "ledctrl.h"

void toggleled ( int ttyfd, unsigned char led, unsigned char status ) {
  unsigned char savedleds;  /* saved led status */
  unsigned char tempbyte;
  ioctl ( ttyfd, KDGETLED, &savedleds );
  if ( status == LEDON ) {
    ioctl ( ttyfd, KDSETLED, savedleds|led );
  } else {
    tempbyte = 0xFF;
    tempbyte ^= led;
    ioctl ( ttyfd, KDSETLED, savedleds&tempbyte );
  }
}

void show_interfaces ( void ) {
  FILE *fp = NULL;
  char line[256];
  
  fp = fopen ( "/proc/net/dev", "r" );
 
  while(!feof(fp)) {
    char *name;
    
    fgets(line, sizeof(line)/sizeof(char), fp);

    /* get the interface name */
    name = strtok(line, " :");
    
    printf("%s ", name);
  }
  fclose(fp);
  
  printf("\n");
}