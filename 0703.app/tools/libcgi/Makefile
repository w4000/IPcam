#DEBUG = 1

CROSS_COMPILE=arm-ca9-linux-gnueabihf-

#------------------------------------------------------------------------------
# Define target
#------------------------------------------------------------------------------
TARGET		:=libcgi.so

#------------------------------------------------------------------------------
# Define objects & sources
#------------------------------------------------------------------------------
OBJS		:= \
	base64.o \
	cgi.o \
	cookie.o \
  	error.o \
	general.o \
	list.o \
	md5.o \
	session.o \
	string.o

SRCS		= $(OBJS:.o=.c)

#------------------------------------------------------------------------------
# Define programs
#------------------------------------------------------------------------------
CC		    :=$(CROSS_COMPILE)gcc
LD		    :=$(CROSS_COMPILE)gcc
STRIP	    :=$(CROSS_COMPILE)strip

#------------------------------------------------------------------------------
# Define directories
#------------------------------------------------------------------------------
LIBDIR		:=#-L[library path]
INCDIR		:=-I./
INSTALLDIR	:=./

#------------------------------------------------------------------------------
# Define default flags
#------------------------------------------------------------------------------
CFLAGS		:=$(INCDIR)
CFLAGS		+=-Wall
CFLAGS		+=$(MOD_CFLAGS)

LFLAGS		:=$(LIBDIR)
LFLAGS		+=-lpthread
LFLAGS		+=$(MOD_LFLAGS)

#------------------------------------------------------------------------------
# Define option flags
#------------------------------------------------------------------------------
ifdef	DEBUG
CFLAGS		+=$(DBG_CFLAGS)
LFLAGS		+=$(DBG_LFLAGS)
else
CFLAGS		+=-O2
endif

#------------------------------------------------------------------------------
# Define rules
#------------------------------------------------------------------------------
.PHONY: clean

all:		$(TARGET)

$(TARGET):
	$(CC) -fPIC -c $(SRCS)
	$(CC) -shared -Wall -o $@ $(OBJS)
	$(STRIP) $@

install:
	cp -a $(TARGET) $(INSTALLDIR)

clean:
	@rm -f $(TARGET) *~ *~ *.o *.so tags
