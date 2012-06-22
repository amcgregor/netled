#
# Makefile - NetLED Make File
#
# make            - By default, make all.
# make all        - Make everything.
# make clean      - Clean up after creating (remove o files, core, debug, etc)
# make force      - Clean up, then make all.
# make install    - Install the binary (and make it if you haven't already)
# make installrc  - Install the rc.d files (startup/shutdown) (REDHAT ONLY)
# make update     - Download the update patch and inflict it on your computer.
#

# non-debug version
CFLAGS    =-Wall -O2 -g -DNDEBUG
LFLAGS    =-s
CC        =gcc

# debug version
# CFLAGS  =-Wall -O2 -DDEBUG
# LFLAGS  =-g
# CC      =gcc

#OBJS      =if.o utils.o config.o daemon.o ledctrl.o netled.o
OBJS      =if.o utils.o daemon.o ledctrl.o netled.o
TARGET    =netled
VERSION   =3.0
LOCATION  =/usr/local/bin
MANPATH   =/usr/local/man/man1

all: $(OBJS) $(TARGET)

$(TARGET): $(OBJS)
	@echo "[m$(CC) $(LFLAGS) -o $(TARGET) $(OBJS)[1;31m"
	@$(CC) $(LFLAGS) -o $(TARGET) $(OBJS)
	@echo -n '[m' 

force: clean $(TARGET)

install: all
	install $(TARGET) $(LOCATION)/$(TARGET) --mode 6755
	install netled.conf /etc/netled.conf
	install netled.1 $(MANPATH)/netled.1

installrc:
	cp -upr rc.d /etc/rc.d

clean:
	rm -f $(OBJS) $(TARGET) 
	rm -f core debug

.c.o: $<
	@echo "[m$(CC) $(CFLAGS) -o $@ -c $<[1;31m"
	@$(CC) $(CFLAGS) -o $@ -c $<
	@echo -n '[m'
