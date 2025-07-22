##################################################################
#
# Makefile for remote setup.
#
##################################################################

include $(TOP_DIR)/Makefile.param

RELVER = $(MK_SYS_VER1)
MAJVER = $(MK_SYS_VER2)
MINVER = $(MK_SYS_VER3)
BUILDVER = $(MK_SYS_VER4)
REVISION = $(MK_SYS_VER5)
RELDATE = $(RELEASE_DATE)

########################################################################
# ARCH_ARM type
# ARM type = ARCH_ARM
# X86 type = ARCH_X86
ARCH_TYPE = ARCH_ARM
ADD_MACRO += -D$(ARCH_TYPE) -D_GNU_SOURCE -DCAM_MAX=$(CAM_MAX)

MACRO += -D$(ARCH_TYPE)

ifeq ($(TOP_DIR),)
	TOP_DIR = ../../../
endif

include ../../../Makefile.param
include ../../../Makefile.pretty

AS      =$(CROSS_COMPILE)as
LD      =$(CROSS_COMPILE)ld
CC      =$(CROSS_COMPILE)gcc
CPP     =$(CC) -E
AR      =$(CROSS_COMPILE)ar
NM      =$(CROSS_COMPILE)nm
STRIP   =$(CROSS_COMPILE)strip
OBJDUMP =$(CROSS_COMPILE)objdump

CROSS_NAME = $(CROSS:gnueabihf-=gnueabihf)

CFLAGS += -Wall -Os
CFLAGS += -D'RELVER="$(RELVER)"' -D'MAJVER="$(MAJVER)"' -D'MINVER="$(MINVER)"' -D'RELDATE="$(RELDATE)"'
CFLAGS += -D'BUILDVER=$(BUILDVER)' -D'REVISION="$(REVISION)"' -D'VER_STR="$(RELVER).$(MAJVER).$(MINVER)-$(REVISION)"'
CFLAGS += $(ADD_MACRO)
CFLAGS += -I../../../ -I../../../include -I../../ -I./include -L./lib
CFLAGS += -I../../fastcgi/include -L../../fastcgi/lib/$(CROSS_NAME)
CFLAGS += -I../../../encoder/
CFLAGS += -I../../../encoder/$(ARCH_PATH)
CFLAGS += -I../../../lib/
CFLAGS += -I../../../ipc/
CFLAGS += -I../../../setup/
CFLAGS += -I../../../_config/
CFLAGS += -I../../../extlib/include
CFLAGS += -Wno-unused-but-set-variable -Wno-implicit-function-declaration
CFLAGS += -DEXT_BUILD
CFLAGS += -I../../../_sdk/$(NARCH)/include/hisilicon
EXT_LIBS = -L../../../extlib/lib/$(CROSS_NAME) -ljansson -luuid -lssl -lcrypto -lmxml 
SETUP_INI = ../../../setup/setup_ini.c
INI_IMPL = ../../../lib/ini_impl.c

TOP_DIR = $(shell pwd)

FCGI_TARGETS = fcgi_api

FCGI_OBJS = fcgi_common.o \
			api_system_information.o 


FCGI_OBJS += api_video_quality_iq.o \
			 api_video_quality_ae.o \
			 api_video_quality_blc.o \
			 api_video_quality_awb.o \
			 api_video_quality_dn.o



ENCODER_DEF = ../../../encoder/$(ARCH_PATH)/video_default.c
VERIFY_USER = ../../../lib/verify_user.c
SHA256 = ../../../lib/sha256.c
ifeq ($(USE_DF2_SUPPORT), 1)
else
CGI_MODEL_SPEC = ../../../_config/model_spec.c
endif
CGI_MODEL_SPEC = ../../../_config/model_spec.c
SETUP_ENC = ../../../setup/setup_encryption.c
IPC_STATUS = ../../../ipc/ipc_status.c
IPC_CLIENT = ../../../ipc/ipc_client.c


all: $(FCGI_TARGETS)

$(FCGI_TARGETS) : dep $(FCGI_OBJS)
	@$(call run_and_test,$(CC) $(CFLAGS) $(INC) $(LDFLAGS) $(MACRO) -o $@.fcgi $@.c $(ENCODER_DEF) $(VERIFY_USER) $(SHA256) ${INI_IMPL} ${SETUP_INI} $(SETUP_ENC) $(IPC_CLIENT) $(IPC_STATUS) $(CGI_MODEL_SPEC) $(EXT_LIBS) -L. -lrt -lstdc++ $(FCGI_OBJS) ../../fastcgi/lib/$(CROSS_NAME)/libfcgi.a ../../../extlib/lib/$(CROSS_NAME)/libaes.a, Building)
	$(STRIP) $@.fcgi
	chmod 755 $@.fcgi

$(FCGI_OBJS): %.o: %.c
	@$(call run_and_test,$(CC) -c -o $@ $< $(CFLAGS) $(INC) $(MACRO), Compiling)

dep :

clean :
	rm -f *.o
	rm -f *.cgi
	rm -f *.so
