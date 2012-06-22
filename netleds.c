/* 
Last updated: Tue May 04 14:11:07 EDT 1999    
*/  

#include <linux/kd.h>
#include <linux/types.h>
#include "netleds.h"

void scrollon(int ttyfd)
{
    unsigned char savedleds; 	/* saved led states */

    ioctl(ttyfd,KDGETLED,&savedleds);
    ioctl(ttyfd,KDSETLED,savedleds^LED_SCR);
}

void scrolloff(int ttyfd)
{
    unsigned char savedleds;

    ioctl(ttyfd,KDGETLED,&savedleds);
    ioctl(ttyfd,KDSETLED,savedleds&~LED_SCR);
}

void capson(int ttyfd)
{
    unsigned char savedleds;

    ioctl(ttyfd,KDGETLED,&savedleds);
    ioctl(ttyfd,KDSETLED,savedleds^LED_CAP);
}

void capsoff(int ttyfd)
{
    unsigned char savedleds;

    ioctl(ttyfd,KDGETLED,&savedleds);
    ioctl(ttyfd,KDSETLED,savedleds&~LED_CAP);
}

void numon(int ttyfd)
{
    unsigned char savedleds;

    ioctl(ttyfd,KDGETLED,&savedleds);
    ioctl(ttyfd,KDSETLED,savedleds^LED_NUM);
}

void numoff(int ttyfd)
{
    unsigned char savedleds;

    ioctl(ttyfd,KDGETLED,&savedleds);
    ioctl(ttyfd,KDSETLED,savedleds&~LED_NUM);
}
