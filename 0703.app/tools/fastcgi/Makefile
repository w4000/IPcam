CROSS_COMPILE = arm-ca9-linux-gnueabihf

AS      =$(CROSS_COMPILE)as
LD      =$(CROSS_COMPILE)ld
CC      =$(CROSS_COMPILE)gcc
CPP     =$(CC) -E
AR      =$(CROSS_COMPILE)ar
NM      =$(CROSS_COMPILE)nm
STRIP   =$(CROSS_COMPILE)strip
OBJDUMP =$(CROSS_COMPILE)objdump

CFLAGS += -Wall -Os
CFLAGS += -I./include -L./lib
LDFLAGS = -lfcgi -lm

TARGET = test


##################################################################

all: $(TARGET)

$(TARGET) :
	$(CC) $(CFLAGS) $(INC) $(LDFLAGS) -o $@.fcgi $@.c
	$(STRIP) $@.fcgi
	chmod 755 $@.fcgi

clean :
	rm -f *.o
	rm -f *.fcgi
