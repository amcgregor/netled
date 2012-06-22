#
# Makefile - NetLED Make File
#
# make
# make clean    - Clean up after creating (remove o files, core, debug, etc)
# make install  - Install the binary (and make it if you haven't already)
#

exe = netled
app = netled.c
led = netleds.c
location = /usr/local/bin
gcc = gcc -DLINUX 

all:	$(exe)

netled: netleds.o netled.o
	$(gcc) -O9	$(app) $(led)	-o netled

netleds.o: netleds.c netleds.h
	$(gcc) -O9 -c	$(led)		-o netleds.o

netled.o: netled.c netleds.h
	$(gcc) -O9 -c	$(app)		-o netled.o

install:
	install $(exe) $(location)/$(exe) --mode 6755
	install netled.conf /etc/netled.conf
	install netled.1 /usr/local/man/man1/netled.1

clean: 
	rm -f netled
	rm -f *.o
	rm -f *.out
	rm -f core debug
