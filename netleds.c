#include <linux/kd.h>
#include <linux/types.h>
#include "netleds.h"

unsigned char savedleds = 0; 	/* saved led states */

void scrollon(void)
{
    ioctl(ttyfd,KDGETLED,&savedleds);
    ioctl(ttyfd,KDSETLED,savedleds^LED_SCR);
}

void scrolloff(void)
{
    ioctl(ttyfd,KDGETLED,&savedleds);
    ioctl(ttyfd,KDSETLED,savedleds&~LED_SCR);
}

void capson(void)
{
    ioctl(ttyfd,KDGETLED,&savedleds);
    ioctl(ttyfd,KDSETLED,savedleds^LED_CAP);
}

void capsoff(void)
{
    ioctl(ttyfd,KDGETLED,&savedleds);
    ioctl(ttyfd,KDSETLED,savedleds&~LED_CAP);
}

void numon(void)
{
    ioctl(ttyfd,KDGETLED,&savedleds);
    ioctl(ttyfd,KDSETLED,savedleds^LED_NUM);
}

void numoff(void)
{
    ioctl(ttyfd,KDGETLED,&savedleds);
    ioctl(ttyfd,KDSETLED,savedleds&~LED_NUM);
}
