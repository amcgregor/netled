/*
Begin3
Title:          NetLED 
Version:        2.0 
Entered-date:   May 29th 1999 
Description:    NetLED monitors the RD and SD of interfaces using the LEDs on your keyboard. 
Keywords:       network, ethernet, ppp, slip, Linux, freeware, keyboard, monitor
Author:         alice@gothcandy.com (Alice Bevan-McGregor)
Maintained-by:  alice@gothcandy.com (Alice Bevan-McGregor)
Primary-site:   https://github.com/amcgregor/netled/
Original-site:  https://github.com/amcgregor/netled/
Platform:       Linux
Copying-policy: GPL
End
*/

/* Daemon/kill support added by Kenneth Skaar
   (kenneths@regina.uio.no) on 25th of May, 1999 */
   
#include <fcntl.h>
#include <linux/kd.h>
#include <linux/types.h>
#include <signal.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "netleds.h"

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
  /* Install SIGTERM handler, so we remove pid file if killed... */
       
  
}




int main(int argc, char *argv[])		
{
    
    /* variables */
    FILE *F1 = NULL;                        /* file in proc fs */
    char tty[15] = "/dev/";                 /* device name */
    int packets[2] = { 0, 0 };              /* packets, duh */
    static int last_packets[2] = { 0, 0 };  /* time of last packet */
    int dummy;                              /* dummy variable */
    char line[256] = { '\0' };              /* put comment here */
    char face[256];                         /* put comment here */
    char *word = NULL;                      /* put comment here */
    char *colon;                            /* interface name delimiter */
    int waitdelay;                          /* How long do I wait? */
    int ttyfd;                              /* The TTY to write to WAS global */
    int daemon=0;                           /* Are we running as a daemon.. */
    int skip=0;
    
    /*Some quick arg processing, check if we should be deamon or take out a daemon*/
    for(dummy=1; dummy<argc; dummy++){
      if(strcmp(argv[dummy],"-k")==0){
	/*we reuse dummy, wont need it after this...*/
	dummy=get_pid();
	if(dummy>0){/*passing an int <= 0 to kill() does a lot more than we want*/
	  if(kill(dummy,SIGTERM)!=0){
	    fprintf(stderr,"Could not kill process %i\n", dummy);
	  }
	} else {
	  fprintf(stderr, "No netled process running..\n");
	}
	exit(0);
      }
      if(strcmp(argv[dummy],"-d")==0){
	daemon=1;
      }
    }
    
    /* start up message / syntax */
    if(daemon==0){ /*If we are a daemon we run in silent mode...*/
      fprintf(stderr, "\n NetLED Version 2.0 ");
      fprintf(stderr, "(see COPYING for lisence information)\n\n");
    }
    if (argc<2) {
        fprintf(stderr, "   netled useage: netled <console> <interface> <refresh> [-k]\n");
        fprintf(stderr, "    console    Select a console to flash the leds on.\n");
        fprintf(stderr, "    interface  Select a running interface to monitor.\n");
        fprintf(stderr, "    refresh    Select a refresh rate, default is 100.\n");
	fprintf(stderr, "    -d         Run as daemon, write pid to /var/run/netled.pid\n");
	fprintf(stderr, "    -k         Kill the running deamon.\n");
        fprintf(stderr, "               Not yet implemented, please do not use.\n\n");
        exit(1);
    }

    if (strlen(argv[1]) >= 10) {
	fprintf(stderr, " Invalid argument: Argument too long!");
	exit (1);
    }

    /* error handle */
    strcat(tty,argv[1]);
    if((ttyfd = open(tty,O_RDWR)) < 0) {
        fprintf(stderr," Error opening keyboard %s\n ",tty); 
        exit(1);
    }
    
    if (!argv[3]) {
        waitdelay = 100;
	if(daemon==0){
	  fprintf(stderr," No refresh specified, using default of %d.", waitdelay);
	}
    } else {
        waitdelay = atoi(argv[3]);
	if(daemon==0){
	  fprintf(stderr," Refresh of %d specified.", waitdelay);
	}
    }
	
    /*Ok arguments and all read, things are up to speed, lets fork off...*/
    if(daemon!=0){
      daemonize();
    }


    /* main program loop */
    while (1) {
        /* open proc file */
        F1 = fopen("/proc/net/dev", "r");
        if (F1 == NULL) {
            fprintf(stderr,"Error using proc "); 
            exit(1);
        }

        strcpy(face, "");
        while (1) {
            fgets(line, 256, F1);
            if (feof(F1)) break;
            word = strtok(line, " ");
            if (word != NULL) strcpy(face, word);
            colon = strchr(face, ':');
            if(colon) *colon = '\0';
            if (strcmp(argv[2], face) == 0) {
                word = strtok(NULL, " ");
                if (word != NULL) sscanf(word, "%d", &packets[0]);
                word = strtok(line+64, " "); 
		word = strtok(NULL, " ");
            	for (skip=0; skip < 5; skip++) word = strtok(NULL, " ");
            	if (word != NULL) sscanf(word, "%d", &packets[1]);
		if (packets[0]!=last_packets[0]) capson(ttyfd); else capsoff(ttyfd);
                if (packets[1]!=last_packets[1]) scrollon(ttyfd); else scrolloff(ttyfd);
                last_packets[0] = packets[0];
                last_packets[1] = packets[1];
            }
        }
	
        /* delay */
        fclose(F1);
        sleep(waitdelay);
    }
}
