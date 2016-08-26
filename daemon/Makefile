CC = gcc
CFLAGS = -Wall -DVERSION="\"`cat VERSION`\""
LDFLAGS = 

prefix=/usr/local/deflect/ansible/autodeflect

PROGRAMS = ad2p autodctl 
C_PROGRAMS = ad2p 

ADMIN_PROGRAMS = autodctl

C_OBJS = cdb.o loadchk.o pid.o signal.o config.o lib.o

ADMIN_OBJS = pid.o config.o

C_COMMON_PREREQ = cdb loadchk pid signal config lib

ADMIN_PREREQ = pid config


all: $(PROGRAMS)

script_programs: $(C_PROGRAMS)

admin_programs: $(ADMIN_PROGRAMS)

ad2p: $(C_COMMON_PREREQ)
	$(CC) $(CFLAGS) -c ad2p.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(C_OBJS) ad2p.o -o ad2p

loadchk:
	$(CC) $(CFLAGS) -c loadchk.c

pid:
	$(CC) $(CFLAGS) -c pid.c

signal:
	$(CC) $(CFLAGS) -c signal.c

config:
	$(CC) $(CFLAGS) -c config.c

lib:
	$(CC) $(CFLAGS) -c lib.c

cdb:
	$(CC) $(CFLAGS) -c cdb.c

autodctl: $(ADMIN_PREREQ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(ADMIN_OBJS) autodctl.c -o autodctl

clean:
	rm -f core *.o *.c~ *.h~

distclean: clean
	rm -f $(PROGRAMS)

install: $(PROGRAMS)  
	install -m 0755 autodctl $(prefix)/bin
	install -m 0755 ad2p $(prefix)/bin
    
.PHONY: install

