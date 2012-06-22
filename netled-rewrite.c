/*
   NetLED rewrite by Michael Vines - based on NetLED 1.3
   (email: mjvines@undergrad.math.uwaterloo.ca)
   
   May be copied or modified under terms of the 
   GNU General Public License
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h> 
#include <fcntl.h>
#include <linux/kd.h>
#include <linux/types.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

const char *NETFILE = "/proc/net/dev";
const char *NETLED_VERSION_STRING = "NetLED 1.3 (rewrite)";

void ledon(int ttyfd, int ledcode) 
{ 
  unsigned char savedleds; 
  if(ledcode == -1)
    return;
  ioctl(ttyfd,KDGETLED,&savedleds); 
  ioctl(ttyfd,KDSETLED,savedleds|ledcode); 
} 

void ledoff(int ttyfd, int ledcode)  
{  
  unsigned char savedleds; 
  if(ledcode == -1)
    return;
  ioctl(ttyfd,KDGETLED,&savedleds); 
  ioctl(ttyfd,KDSETLED,savedleds&~ledcode); 
}  

/* we do this in a couple places, so might as well
   try to conserve code space */
FILE *open_netfile(void) 
{
  FILE *fp = NULL;
  char line[256];
  
  if( NULL == (fp = fopen(NETFILE, "r"))) {
    printf("Error: Unable to open %s: is /proc mounted?\n", NETFILE);
    exit(1);
  }

  /* Make sure /proc is sane...check for header info */
  fgets(line, sizeof(line)/sizeof(char), fp);
  if( 0 != (strncmp(line, "Inter-|", 7))) {
    printf("Error: %s is in an unknown format\n", NETFILE);
    exit(1);
  }
  fgets(line, sizeof(line)/sizeof(char), fp);
  if( 0 != (strncmp(line, " face |", 7))) {
    printf("Error: %s is in an unknown format\n", NETFILE);
    exit(1);
  }
  
  return fp;
} /* open_netfile() */


/* check if 'interface' is a valid interface */
int is_interface_valid(char *interface) 
{
  FILE *fp = NULL;
  char line[256];
  
  fp = open_netfile();

  while(!feof(fp)) {
    char *name;

    fgets(line, sizeof(line)/sizeof(char), fp);

    /* get the interface name */
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
} /* is_interface_valid() */


/* update the interfaces' rxcount and txcount with the latest values */
void update_stats(char *interface_strlst, 
    unsigned long *rxcount, unsigned long *txcount) 
{
  FILE *fp = NULL;
  char line[256];
  
  fp = open_netfile();
  
  *rxcount = 0;
  *txcount = 0;
  while(!feof(fp)) {
    char *interface_name;
    char *token;
    int i;
    unsigned long curr_rxcount = 0, curr_txcount = 0;

    fgets(line, sizeof(line)/sizeof(char), fp);
    
    /* get the interface name */
    interface_name = strchr(line, ':');
    if(interface_name == NULL) 
      break;
    *interface_name = '\0';
    
    /* get the receive bytes */
    token = strtok(interface_name+1, " ");
    if(token != NULL) 
      curr_rxcount = atoi(token);
    
    /* skip to the transmit bytes */
    for(i = 0; i < 8; i++)
      token = strtok(NULL, " ");

    if(token != NULL)
      curr_txcount = atoi(token);
    
    /* trim leading spaces */
    for(interface_name = line; *interface_name == ' '; interface_name++);
    
    if((interface_strlst == NULL) || 
       (strstr(interface_strlst, interface_name) != NULL)) {
      *rxcount += curr_rxcount;
      *txcount += curr_txcount;
    }
  }
  fclose(fp);
} /* update_stats */


/* print all the interfaces we know about */
void show_interfaces(void) 
{
  FILE *fp = NULL;
  char line[256];
  
  fp = open_netfile();
 
  while(!feof(fp)) {
    char *name;
    
    fgets(line, sizeof(line)/sizeof(char), fp);

    /* get the interface name */
    name = strtok(line, " :");
    
    printf("%s ", name);
  }
  fclose(fp);
  
  printf("\n");
} /* show_interfaces() */

void show_help(void) 
{
  printf("NetLED v1.3 (rewrite)\n(see COPYING for license information)\n\n");
  printf("Usage: netled [options]\n");
  printf("Options:\n");
  printf("  -t, --tty=[tty]              which tty leds to flash, use\n");
  printf("                               --tty=console for every tty\n");
  printf("                               (default=console)\n");
  printf("  -r, --refresh=[ms]           refresh rate in milliseconds \n");
  printf("                               (default=100)\n");
  printf("  -R, --rx=[led]               set the led to flash for RX\n");
  printf("                               led=off,caps,num,scroll\n");
  printf("                               (default=scroll)\n");
  printf("  -T, --tx=[led]               set the led to flash for TX\n");
  printf("                               led=off,caps,num,scroll\n");
  printf("                               (default=scroll)\n");
  printf("  -i, --interface=[interface]  specify interface to monitor, use\n");
  printf("                               --interface=all to monitor all\n");
  printf("                               interfaces. multiple interfaces can\n");
  printf("                               be comma delimited\n");
  printf("                               ie. --interface=eth0,eth1\n");
  printf("                               (default=all)\n");
  printf("  -l, --list                   list available interfaces\n");
  printf("  -h, --help                   display this help and exit\n");
  printf("  -v, --version                display version info and exit\n\n");
} /* show_help() */

int main(int argc, char *argv[])		
{
  unsigned long rxcount, txcount;           /* receive and transmit count */
  unsigned long prev_rxcount, prev_txcount; /* previous values */
  int rx_led = LED_SCR;
  int tx_led = LED_SCR;
  int waitdelay = 100;            /* How long do I wait? (in ms) */
  char *interface_strlst = NULL;  /* string list of which interfaces to monitor
                                      ie. "eth0,eth1,lo" */
  char tty[15] = "/dev/console";  /* default to the console */
	int ttyfd;                      /* tty file descriptor */ 
  
  /* process command line arguments */
  while(1) {
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
      {0, 0, 0, 0}
    };
     
    c = getopt_long(argc, argv, "t:i:lr:hvT:R:", 
        long_options, &option_index);
    
    if(c == -1)
      break;
    
    switch(c) {
      /* --tx */
      case 'T':
        if(strcasecmp(optarg, "off") == 0) {
          tx_led = -1;
        } else if(strcasecmp(optarg, "caps") == 0) {
          tx_led = LED_CAP;
        } else if(strcasecmp(optarg, "num") == 0) {
          tx_led = LED_NUM;
        } else if(strcasecmp(optarg, "scroll") == 0) {
          tx_led = LED_SCR;
        } else {
          printf("Error: invalid --tx option: %s\n", optarg);
          exit(1);
        }
        break;

      /* --rx */
      case 'R':
        if(strcasecmp(optarg, "off") == 0) {
          rx_led = -1;
        } else if(strcasecmp(optarg, "caps") == 0) {
          rx_led = LED_CAP;
        } else if(strcasecmp(optarg, "num") == 0) {
          rx_led = LED_NUM;
        } else if(strcasecmp(optarg, "scroll") == 0) {
          rx_led = LED_SCR;
        } else {
          printf("Error: invalid --rx option: %s\n", optarg);
          exit(1);
        }
        break;

      /* --tty */
      case 't':
        strcpy(tty, "/dev/");
        strncat(tty, optarg, sizeof(tty)/sizeof(char) - 5);
        break;
        
      /* --interface */
      case 'i':
        /* interface_strlst = NULL if we are monitoring everything */
        if(strstr(optarg, "any")) {
          interface_strlst = NULL;
          break;
        }
         
        interface_strlst = malloc((strlen(optarg)+2)*sizeof(char));
         
        strcpy(interface_strlst, optarg);
        break;
        
      /* --refresh */
      case 'r':
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
        
      /* --list */
      case 'l': 
        printf("Available interfaces: ");
        show_interfaces();
        exit(0);
        
      /* --version */
      case 'v':
        printf("%s\n", NETLED_VERSION_STRING);
        exit(0);

      /* --help */
      case 'h':
        show_help();
        exit(0);
        
      default:
        printf("Error: Unexpected getopt() return value: %c\n", c);
        exit(1);
    }
  }
  
  /* check for invalid interfaces, not the best way to do it but not a  
     big deal cause we'll just be executing this code once  */
  if(interface_strlst != NULL) {
    char *token;
    
    token = strtok(interface_strlst, ",");
    while(token != NULL) {
      if(!is_interface_valid(token)) {
        printf("Error: Unknown interface %s, "
               "use the -l option to view available interfaces\n", token);
        exit(1);
      }
      token = strtok(NULL, ","); 
    }
  }
  
  /* open the tty */
  if((ttyfd = open(tty,O_RDWR)) < 0) {
      printf("Error: Unable to open tty: %s\n", tty);
      exit(1);
  }

  /* main monitor loop */
  update_stats(interface_strlst, &rxcount, &txcount);
  while(1) {
    prev_rxcount = rxcount;
    prev_txcount = rxcount;
    update_stats(interface_strlst, &rxcount, &txcount);
    
    /* check if the tx and rx leds are the same */
    if(tx_led == rx_led) {
      if((prev_txcount != txcount) || (prev_rxcount != rxcount))
        ledon(ttyfd, tx_led);
      else 
        ledoff(ttyfd, tx_led);
    } else { 
      if(prev_txcount != txcount)
        ledon(ttyfd, tx_led);
      else 
        ledoff(ttyfd, tx_led);
        
      if(prev_rxcount != rxcount) 
        ledon(ttyfd, rx_led);
      else 
        ledoff(ttyfd, rx_led);
    }
    usleep(waitdelay*1000);
  }
} /* main() */

