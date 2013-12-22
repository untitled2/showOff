#
# Asiakas / palvelin makefile
#
# make palvelin
# make asiakas
#
CCBIN=gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -funsigned-char
CFLAGS+= -posix

# math
LDFLAGS += -lgmp
NETTLE=/usr/local/lang/nettle-2.5-pre
CFLAGS += -I$(NETTLE)/include

ifeq ($(shell uname -i), x86_64) # onko 64-bit
LDFLAGS += -L$(NETTLE)/lib64 -lnettle
else
LDFLAGS += -L$(NETTLE)/lib -lnettle
endif

CC= $(CCBIN)

.PHONY: all clean 

#all: asiakas.o palvelin.o
#	$(CC) -o $@  $^ $(LDFLAGS)

asiakas: asiakas.o util.o srp.o hajap.o
	$(CC) -o $@  $^ $(LDFLAGS)

palvelin: palvelin.o util.o srp.o hajap.o
	$(CC) -o $@  $^ $(LDFLAGS)

srp.o : srp.c
util.o : util.c
hajap.o : hajap.c

dist:
	( cd ..; zip -r htyo1-socket.zip htyo1-socket/ )
clean:
	rm -f asiakas palvelin *~ *.o


# --------------------------------------------------------------------
