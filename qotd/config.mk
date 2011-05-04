# qotd version
VERSION = 0.1

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# includes and libs
INCS = -I. -I/usr/include
LIBS = -L/usr/lib -lc

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\" -D_POSIX_C_SOURCE=2

# uncomment this to change the renewal time (in seconds).
#CPPFLAGS += -DPERIOD_RENEWAL=30

CFLAGS += -std=c99 -pedantic -Wall ${INCS} ${CPPFLAGS}
LDFLAGS += -s ${LIBS}

# compiler and linker
CC ?= cc
