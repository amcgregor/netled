#
# Makefile - NetLED Make File
#
# make          - By default, make all.
# make all      - Make everything.
# make clean    - Clean up after creating (remove o files, core, debug, etc)
# make install  - Install the binary (and make it if you haven't already)
# make update   - Download the update patch and inflict it on your computer.
#

EXE = netled
LOCATION = /usr/local/bin
CC = gcc -DLINUX 
OBJS = netled.o
VERSION = 3.0

all: netled

netled: $(OBJS)

netled.o: netled.c

install:
	install $(EXE) $(LOCATION)/$(EXE) --mode 6755
	install netled.conf /etc/netled.conf
	install netled.1 /usr/local/man/man1/netled.1

clean: 
	rm -f netled
	rm -f *.o
	rm -f *.out
	rm -f core debug
