# swm version
VERSION = 1.0.0

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# freetype
FREETYPELIBS = -lfontconfig -lXft
FREETYPEINC = /usr/include/freetype2

# OpenBSD (uncomment)
#FREETYPEINC = ${X11INC}/freetype2
#MANPREFIX = ${PREFIX}/man

# includes and libs
INCS = -I${X11INC} -I${FREETYPEINC}
LIBS = -L${X11LIB} -lX11 ${FREETYPELIBS}

# flags
DEBUG ?= 0
ARGUMENT_FLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -DVERSION=\"$(VERSION)\" -DDEBUG=$(DEBUG)

ifeq ($(DEBUG), 0)
	CFLAGS = -std=c99 -pedantic -Wall -Wno-deprecated-declarations -Os ${INCS} ${ARGUMENT_FLAGS}
else
	CFLAGS = -g -std=c99 -pedantic -Wall -O0 ${INCS} ${ARGUMENT_FLAGS}
endif

LDFLAGS = ${LIBS}
