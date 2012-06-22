/*
Begin3
Title:          NetLED
Version:        1.2
Entered-date:   May 12th 1999
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
    char tty[15] = "/dev/";			/* device name */
    int packets[2] = { 0, 0 };			/* packets, duh */
    static int last_packets[2] = { 0, 0 };	/* time of last packet */
    int skip = 0, dummy;			/* dummy variable */
    char line[256] = { '\0' };			/* put comment here */
    char face[256];				/* put comment here */
    char *word = NULL;				/* put comment here */
    int ref;					/* put comment here */
    
    /* start up message / syntax */
    fprintf(stderr, "\nNetLED Version 1.2 ");
    fprintf(stderr, "(see COPYING for lisence information)\n\n");
    if (argc<2) {
        fprintf(stderr, "  netled useage: netled <console> <interface> <refresh> [-k]\n");
        fprintf(stderr, "    console	Select a console to flash the leds on.\n");
        fprintf(stderr, "    interface	Select a running interface to monitor.\n");
        fprintf(stderr, "    refresh	Select a refresh rate, default is 100.\n");
	fprintf(stderr, "    -k		Kill the running deamon.\n");
        fprintf(stderr, " 		Not yet implemented, please do not use.\n\n");
        exit(1);
    }

    if (strlen(argv[1]) >= 10) {
	fprintf(stderr, "argument too long");
	exit (1);
    }

    /* error handle */
    strcat(tty,argv[1]);
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
	
        strcpy(face, "");
        while (!feof(F1)) {
	    fgets(line, 256, F1);
	    if (feof(F1)) break;
	    word = (char *)strtok(line, " ");
	    if (word != NULL) strcpy(face, word);
	    face[strlen(face)-1]='\0';
	    word = (char *)strtok(NULL, " ");
	    if (word != NULL) sscanf(word, "%d", &packets[0]);
	    for (skip=0; skip < 5; skip++) word = (char *)strtok(NULL, " ");
	    if (word != NULL) sscanf(word, "%d", &packets[1]);
	    if (strcmp(argv[2], face) == 0) {
	        if (packets[0]!=last_packets[0]) capson(); else capsoff();
	        if (packets[1]!=last_packets[1]) scrollon(); else scrolloff();
	    	last_packets[0] = packets[0];
	        last_packets[1] = packets[1];
	    }
	}
	
	/* delay */
	fclose(F1);
	if (argv[3] = "") usleep(100); else usleep(atoi(argv[3]));
    }
}