#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <linux/kd.h>
#include <linux/types.h>

#define LEDOFF  0
#define LEDON   1

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

/*get pid from /var/run/netled.pid*/
int get_pid(void){
  int pid;
  FILE *PID = NULL;
  char line[256];

  PID=fopen("/var/run/netled.pid","r");
  if(NULL==PID||feof(PID)){
    return -1;
  }
  while(!feof(PID)){
    fgets(line, 255, PID);
    pid=atoi(line);
    if(pid>1){
      fclose(PID);
      return pid;
    }
  }
  fclose(PID);
  return -1;
}

/*write pid to /var/run/netled.pid*/
void put_pid(pid_t pid){
  FILE *PID = NULL;
  PID=fopen("/var/run/netled.pid","w");
  if(PID==NULL){
    fprintf(stderr,"Coulndt open pid file for writing\n");
  }
  fprintf(PID,"%i\n",pid);
  fclose(PID);
}

/* comment goes here, but you should know what this does */
void daemon_kill(int sig){
  unlink("/var/run/netled.pid");
  exit(0);
}

/*fork a child, and exit parent if succesfull...*/
void daemonize(void){
  pid_t me;				    /* pid of process...*/
  int oldpid;
  /* Check if allready running, if so dont start another */
  oldpid=get_pid();
  if(oldpid!=-1){
    fprintf(stderr,"netled allready running with pid %i\n",oldpid);
    exit(1);
  }
  /* Install signal handler before we fork */
  signal(SIGTERM,&daemon_kill);
  /*  we start off by forking ourself... */
  me=fork();
  if(0!=me){
    /*in parent...did things go ok?*/
    if(-1!=me){
      /*things went ok, write out pid and exit*/
      put_pid(me);
      exit(0);
    } else {
      /*whoops error...*/
      fprintf(stderr, "Problem forking..");
      exit(1);
    }
  }
}

/* main application procedure */
int main ( int argc, char **argv ) {
  // variables
  FILE *procnet = NULL;
  char ttyname[20] = "/dev/";
  int ttyfd;
  char line[255];
  char name[16];
  char recieved[16];
  char oldrecieved[16];
  char transmit[16];
  char oldtransmit[16];
  int waitdelay = 100;
  int i;
  int daemonized;

  if ( daemonized == 0 ) {
    fprintf ( stderr, "\n NetLED Version 3.0 " );
    fprintf ( stderr, "(see COPYING for lisence information)\n\n" );
  }
    
  if ( argc < 2 ) {
    fprintf ( stderr, "  netled useage: netled <console> <interface> <refresh> [-k]\n" );
    fprintf ( stderr, "    console    Select a console to flash the leds on.\n" );
    fprintf ( stderr, "    interface  Select a running interface to monitor.\n" );
//    fprintf ( stderr, "    refresh    Select a refresh rate, default is 100.\n" );
    fprintf ( stderr, "    -d         Run as daemon, write pid to /var/run/netled.pid\n" );
    fprintf ( stderr, "    -k         Kill the running deamon.\n\n" );
    exit ( 1 );
  }

  if ( strlen ( argv[1] ) >= 10 ) {
    fprintf (stderr, " Invalid argument: Argument too long!" );
    exit ( 1 );
  }

  strcat( ttyname, argv[1]);
  if ( ( ttyfd = open ( ttyname, O_RDWR ) ) < 0) {
    fprintf ( stderr, " Error opening keyboard %s\n ", ttyname); 
    exit ( 1 );
  }

  /*Some quick arg processing, check if we should be deamon or take out a daemon*/
  for(i=1; i<argc; i++){
    if(strcmp(argv[i],"-k")==0){
      /*we reuse dummy, wont need it after this...*/
	i=get_pid();
	if(i>0){/*passing an int <= 0 to kill() does a lot more than we want*/
	  if(kill(i,SIGTERM)!=0){
	    fprintf(stderr,"Could not kill process %i\n", i);
	  }
	} else {
	  fprintf(stderr, "No netled process running..\n");
	}
	exit(0);
      }
      if(strcmp(argv[i],"-d")==0){
	daemonized = 1;
      }
    }

  if ( daemonized != 0 ) daemonize ( );

  while ( 1 ) {
    procnet = fopen ( "/proc/net/dev", "r" );
    if ( procnet == NULL ) {
      perror ( "Error opening /proc/net/dev" );
      exit ( 1 );
    }
    
    fgets ( line, 255, procnet );
    fgets ( line, 255, procnet );

    while ( fgets ( line, 255, procnet ) ) {
     
      memset ( name, 0, sizeof ( name ) );
      strncpy ( name, line, 6 );

      if ( strstr ( name, argv[2] ) ) {
      
        memset ( recieved, 0, sizeof ( recieved ) );
        strncpy ( recieved, line + 15, 8 );

        memset ( transmit, 0, sizeof ( transmit ) );
        strncpy ( transmit, line + 74, 8 ) ;

//        printf ( "[H[Kname: \"%s\"\n", name);        
        if ( strcmp ( recieved, oldrecieved ) ) {
//          printf ( "[Krecieved: \"%s\"\n", recieved );
          toggleled ( ttyfd, LED_CAP, LEDON );
        } else {
	  toggleled ( ttyfd, LED_CAP, LEDOFF );
	}
        if ( strcmp ( transmit, oldtransmit ) ) {
//          printf ( "[Ktransmit: \"%s\"\n", transmit ); 
	  toggleled ( ttyfd, LED_SCR, LEDON );
        } else {
	  toggleled ( ttyfd, LED_SCR, LEDOFF );
	}
   
        strcpy ( oldrecieved, recieved );
        strcpy ( oldtransmit, transmit );

      }
    }

    fclose (procnet);
    usleep (waitdelay * 100);
  }  
  return 0;
}
