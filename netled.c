/*
Begin3
Title:          NetLED 
Version:        1.1 
Entered-date:   April 19th 1999 
Description:    NetLED monitors the RD and SD of interfaces using the LEDs on
                your keyboard. 
Keywords:       network, ethernet, ppp, slip, Linux, freeware, keyboard,
                monitor
Author:         alice@gothcandy.com (Alice Bevan-McGregor)
Maintained-by:  alice@gothcandy.com (Alice Bevan-McGregor)
Primary-site:   https://github.com/amcgregor/netled/
Original-site:  https://github.com/amcgregor/netled/
Platform:       Linux a.out (Kernel 2.0.34 or above?)
Copying-policy: GPL
End
*/

/* 
Last updated: Tue May 04 15:50:24 EDT 1999                                     
*/  

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h> 
#include <fcntl.h>
#include <linux/kd.h>
#include <linux/types.h>
#include <unistd.h>
#include "netleds.h"


int main(int argc, char *argv[])		
{
    /* variables */
    FILE *F1 = NULL;				/* file in proc fs */
    char tty[32] = "/dev/";			/* device name */

    int packets[NUM_DEVICES][2];		/* packets for up to N devices */

    int last_packets[NUM_DEVICES][2];		/* number of packets last time
						   we looked */

    int skip = 0;				/* count of skipped fields */
    char line[256] = "";			/* line read from /proc/net/dev */
    char interface[256];			/* current interface from /proc/net/dev */
    char *word = NULL;				/* current field from /proc/net/dev */
    int iface_num;
    int turn_capson, turn_scrollon;

    int ttyfd;					/* fd for the tty */
    int start_args;				/* where the real arguments start */
    int sleeptime = 100 * 1000;			/* default delay, in microseconds,
						   between polling /proc/net/dev */
    char opt_char;

    /* start up message / syntax */
    fprintf(stderr, "\nNetLED Version 1.1 ");
    fprintf(stderr, "(see COPYING for lisence information)\n\n");
    if (argc<2) {
        fprintf(stderr, "  usage: netled [-k] [-w wait_time] <console> <interface> "
		        "... \n");
	fprintf(stderr, "    -k             Kill the running deamon.\n");
        fprintf(stderr, "                     (Not yet implemented -- do not use!)\n");
        fprintf(stderr, "    -w wait_time   Select how long to pause between checking "
		        "for activity.\n"
			"                   Defaults to 100 milliseconds.\n");
        fprintf(stderr, "    console        Select a console to flash the leds on.\n");
        fprintf(stderr, "    interface      Select running interface(s) (up to %d) "
		        "to monitor.\n",
			NUM_DEVICES);
	fprintf(stderr, "\n" );
        exit(1);
    }

    /* init packets and last_packets */
    for (iface_num = 0; iface_num < NUM_DEVICES; iface_num++)
    {
	packets[ iface_num ][ 0 ] = 0;
	packets[ iface_num ][ 1 ] = 0;
    }

    /* parse arguments */
    while (EOF != (opt_char = getopt( argc, argv, "+kw:" ))) {
	switch (opt_char)
	{
	    case 'w':
		/* convert to microseconds */
		sleeptime = atoi( optarg ) * 1000;
		break;
	    case 'k':
		fprintf(stderr, "-k not supported yet.  Ignoring.\n" );
		break;
	}
    }
    start_args = optind;

    /* did user include "/dev/" in console device name? */
    if (0 == strncmp( argv[ start_args ], "/dev/", 5 ))
	strcpy( tty, argv[ start_args ] );
    else
	strcat(tty,argv[ start_args ]);

    /* did user include "/dev/" in network device name? */
    for (iface_num = start_args; iface_num < argc; iface_num++) {
	if (0 == strncmp( argv[ iface_num ], "/dev/", 5 )) {
	    argv[ iface_num ] += 5;
	}
    }

    /* error handle */
    if((ttyfd = open(tty,O_RDWR)) < 0) {
	fprintf(stderr,"Error opening keyboard %s\n ",tty); 
	exit(1);
    }

    /* main program loop */
    while (1) {
	/* open proc file */
	F1 = fopen("/proc/net/dev", "r");
        if (F1 == NULL) {
	    fprintf(stderr,"Error using proc "); 
	    exit(1);
	}

	/* skip the first two lines -- their just headers */
	fgets(line, 256, F1);
	fgets(line, 256, F1);

	/* do the real work */
        while (!feof(F1)) {
	    fgets(line, 256, F1);
	    if (feof(F1)) break;

	    interface[ 0 ] = 0;
	    word = (char *)strtok(line, " ");
	    if (word != NULL) strcpy(interface, word);

	    interface[strlen(interface)-1]='\0';

	    for (iface_num = 0; 
		 iface_num < argc - start_args
		     && iface_num < NUM_DEVICES; 
		 iface_num++) 
	    {
		if (strcmp(argv[start_args + iface_num + 1], interface) == 0) {
		    word = (char *)strtok(NULL, " ");
		    if (word != NULL) sscanf(word, "%d", &packets[iface_num][0]);
		    for (skip=0; skip < 5; skip++) word = (char *)strtok(NULL, " ");
		    if (word != NULL) sscanf(word, "%d", &packets[iface_num][1]);

		    turn_capson = (packets[iface_num][0]!=last_packets[iface_num][0]);
		    turn_scrollon = (packets[iface_num][1]!=last_packets[iface_num][1]);

		    last_packets[iface_num][0] = packets[iface_num][0];
		    last_packets[iface_num][1] = packets[iface_num][1];
		}
	    }

	    if (turn_capson) 
		capson(ttyfd);
	    else
		capsoff(ttyfd);

	    if (turn_scrollon)
		scrollon(ttyfd);
	    else
		scrolloff(ttyfd);
	}

	fclose(F1);

	/* delay */
	usleep(sleeptime);
    }
}

