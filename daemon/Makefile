CC = gcc
CFLAGS = -O2 -Wall -DVERSION="\"`cat VERSION`\""
LDFLAGS = 
SSL = -lssl -lcrypto
SSH = -lssh2

prefix=..

PROGRAMS = ad2p ad2ssh ad2runner autodeflect 
C_PROGRAMS = ad2p ad2ssh ad2runner

ADMIN_PROGRAMS = autodeflect

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

ad2ssh: $(C_COMMON_PREREQ)
	$(CC) $(CFLAGS) -c ad2ssh.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(C_OBJS) $(SSL) $(SSH) ad2ssh.o -o ad2ssh

ad2runner: $(C_COMMON_PREREQ)
	$(CC) $(CFLAGS) -c ad2runner.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(C_OBJS) $(SSL) $(SSH) ad2runner.o -o ad2runner

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

autodeflect: $(ADMIN_PREREQ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(ADMIN_OBJS) $(SSL) autodeflect.c -o autodeflect

clean:
	rm -f core *.o *.c~ *.h~

distclean: clean
	rm -f $(PROGRAMS)

install: $(PROGRAMS)  
	install -s -m 0755 autodeflect $(prefix)/bin
	install -s -m 0755 ad2p $(prefix)/bin
	install -s -m 0755 ad2ssh $(prefix)/bin
	install -s -m 0755 ad2runner $(prefix)/bin
    
.PHONY: install

