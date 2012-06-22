#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/types.h>

#include "if.h"
#include "config.h"
#include "ledctrl.h"
#include "daemon.h"

const char *NETLED_VERSION_STRING = "NetLED Version 4.0";

void show_help ( void ) {
  fprintf(stderr, "Usage: netled [options]\n");
  fprintf(stderr, "  -t, --tty=[tty]              which tty leds to flash, use\n");
  fprintf(stderr, "                               --tty=console for every tty\n");
  fprintf(stderr, "                               (default=console)\n");
  fprintf(stderr, "  -r, --refresh=[ms]           refresh rate in milliseconds \n");
  fprintf(stderr, "                               (default=100)\n");
  fprintf(stderr, "  -R, --rx=[led]               set the led to flash for RX\n");
  fprintf(stderr, "                               led=off,caps,num,scroll\n");
  fprintf(stderr, "                               (default=config file)\n");
  fprintf(stderr, "  -T, --tx=[led]               set the led to flash for TX\n");
  fprintf(stderr, "                               led=off,caps,num,scroll\n");
  fprintf(stderr, "                               (default=config file)\n");
  fprintf(stderr, "  -i, --interface=[interface]  specify interface to monitor.\n");
  fprintf(stderr, "                               ie. --interface=eth0,\n");
  fprintf(stderr, "                               (default=eth0)\n");
  fprintf(stderr, "  -d, --daemonize              run NetLED as a daemon\n");
  fprintf(stderr, "  -k, --kill                   kill a running NetLED daemon\n");
  fprintf(stderr, "  -q, --query                  find the PID of the NetLED daemon\n");
  fprintf(stderr, "  -l, --list                   list available interfaces\n");
  fprintf(stderr, "  -h, --help                   display this help and exit\n");
  fprintf(stderr, "  -v, --version                display version info and exit\n\n");
}

// main application procedure
int main ( int argc, char **argv ) {
  // variables
  char tty[15] = "/dev/console";  /* default to the console */
  int ttyfd;
 
  int waitdelay = 100, i = 0;
  
  char *interface = "eth0";
  if_throughput_t interdat = { "eth0", 0, 0, 0, 0, 0, 0 };
  int rx_led = LED_SCR;
  int tx_led = LED_SCR;

  /* process command line arguments */
  while ( 1 ) {
    int option_index = 0;
    int c;

    static struct option long_options[] = {
      {"tty", 1, 0, 't'},
      {"interface", 1, 0, 'i'},
      {"list", 0, 0, 'l'},
      {"refresh", 1, 0, 'r'},
      {"help", 0, 0, 'h'},
      {"version", 0, 0, 'v'},
      {"tx", 1, 0, 'T'},
      {"rx", 1, 0, 'R'},
      {"query", 0, 0, 'q'},
      {"daemonize", 0, 0, 'd'},
      {"kill", 0, 0, 'k'},
      {0, 0, 0, 0}
    };
     
    c = getopt_long(argc, argv, "t:i:lr:hvT:R:qdk", 
        long_options, &option_index);
    
    if(c == -1)
      break;
    
    switch(c) {
      case 'T':                                   // --tx
        if(strcasecmp(optarg, "off") == 0) {
          tx_led = -1;
        } else if(strcasecmp(optarg, "caps") == 0) {
          tx_led = LED_CAP;
        } else if(strcasecmp(optarg, "cap") == 0) {
          tx_led = LED_CAP;
        } else if(strcasecmp(optarg, "num") == 0) {
          tx_led = LED_NUM;
        } else if(strcasecmp(optarg, "scroll") == 0) {
          tx_led = LED_SCR;
        } else if(strcasecmp(optarg, "scr") == 0) {
          tx_led = LED_SCR;
        } else {
          printf("Error: invalid --tx option: %s\n", optarg);
          exit(1);
        }
        break;

      case 'R':                                   // --rx
        if(strcasecmp(optarg, "off") == 0) {
          rx_led = -1;
        } else if(strcasecmp(optarg, "caps") == 0) {
          rx_led = LED_CAP;
        } else if(strcasecmp(optarg, "cap") == 0) {
          rx_led = LED_CAP;
        } else if(strcasecmp(optarg, "num") == 0) {
          rx_led = LED_NUM;
        } else if(strcasecmp(optarg, "scroll") == 0) {
          rx_led = LED_SCR;
        } else if(strcasecmp(optarg, "scr") == 0) {
          rx_led = LED_SCR;
        } else {
          printf("Error: invalid --rx option: %s\n", optarg);
          exit(1);
        }
        break;

      case 't':                                   // --tty
        strcpy(tty, "/dev/");
        strncat(tty, optarg, sizeof(tty)/sizeof(char) - 5);
        break;
        
      case 'i':                                   // --interface
        strcpy(interface, optarg);
        break;
        
      case 'r':                                   // --refresh
        waitdelay = (int)strtol(optarg, NULL, 10);
        if(errno == ERANGE) {
          printf("Warning: Invalid refresh rate specified, using 100: %s\n", 
              optarg);
          waitdelay = 100;
        } 
        
        if(waitdelay < 10) {
          printf("Warning: Refresh rate is tiny, using 10 instead: %s\n", 
              optarg);
          waitdelay = 10;
        }
        
        if(waitdelay > 10000) {
          printf("Note: Refresh rate is quite large." 
                 "  Is this really what you want?: %s\n", optarg);
        }
        break;
        
      case 'l':                                   // --list
        printf("Available interfaces: ");
        show_interfaces();
        exit(0);
        
      case 'v':                                   // --version
        printf("%s\n", NETLED_VERSION_STRING);
        exit(0);

      case 'h':                                   // --help
        show_help();
        exit(0);

      case 'd':
	daemonized = 1;
        break;

      case 'k':
	i = get_pid();
	if ( i > 0 ) {
	  if ( kill ( i, SIGTERM ) != 0 ) {
	    fprintf (stderr, "Error: could not kill process %i.\n", i);
	  }
	} else {
	  fprintf(stderr, "Error: no netled process running.\n");
	}
	exit(0);

      case 'q':
        i = get_pid();
	if ( i > 0 ) {
	  fprintf (stderr, "NetLED process running with PID %i.\n", i);
	} else {
	  fprintf (stderr, "Error: no netled process running.\n");
	}
	exit(0);
        
      default:
        printf("Error: Unexpected getopt() return value: %c\n", c);
        exit(1);
    }
  }
  
  /* check for invalid interfaces, not the best way to do it but not a  
     big deal cause we'll just be executing this code once  */
/*  if ( !is_interface_valid(interface) ) {
    printf("Error: Unknown interface %s, "
           "use the -l option to view available interfaces\n", interface);
    exit(1);
  }*/
  
  /* open the tty */
  if ( (ttyfd = open(tty,O_RDWR) ) < 0 ) {
      printf("Error: Unable to open tty: %s\n", tty);
      exit(1);
  }

  if ( daemonized != 0 ) daemonize ( );

  strcpy (interdat.if_name, interface);
  
  while ( 1 ) {
    if (!get_if_throughput(&interdat)) {
      if ( interdat.chg_bytes_in > 0 ) {
        toggleled ( ttyfd, LED_CAP, LEDON );
      } else {
        toggleled ( ttyfd, LED_CAP, LEDOFF );
      }

      if ( interdat.chg_bytes_out > 0 ) {
        toggleled ( ttyfd, LED_SCR, LEDON );
      } else {
        toggleled ( ttyfd, LED_SCR, LEDOFF );
      }
    }

    usleep(waitdelay*1000);
  }  
  return 0;
}
