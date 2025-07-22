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
ADD_MACRO += -D$(ARCH_TYPE) -D_GNU_SOURCE -DCAM_MAX=$(CAM_MAX) -DONVIF_CAM_MAX=$(ONVIF_CAM_MAX)

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
CFLAGS += -I../../../ -I../../../include -I../../ -I./include -L./lib/$(CROSS_NAME)
CFLAGS += -I../../fastcgi/include -L../../fastcgi/lib/$(CROSS_NAME)
CFLAGS += -I../../../encoder/
CFLAGS += -I../../../encoder/$(ARCH_PATH)
CFLAGS += -I../../../lib/
CFLAGS += -I../../../setup/
CFLAGS += -I../../../_config/
CFLAGS += -I../../../extlib/include
CFLAGS += -Wno-unused-but-set-variable -Wno-implicit-function-declaration
CFLAGS += -DEXT_BUILD
CFLAGS += -I../../../_sdk/$(NARCH)/include/hisilicon
CFLAGS += -D'DB_MANAGER_FOR_CGI'

EXT_LIBS = -L../../../extlib/lib/$(CROSS_NAME) -luuid -lssl -lcrypto  -lsqlite3 -ljansson -lmxml
LDFLAGS = -lc -lcgi

TOP_DIR = $(shell pwd)

FCGI_TARGETS = fcgi_status
FCGI_OBJS = fcgi_session.o \
			fcgi_common.o


ENCODER_DEF = ../../../encoder/$(ARCH_PATH)/video_default.c
VERIFY_USER = ../../../lib/verify_user.c
SHA256 = ../../../lib/sha256.c
ifeq ($(USE_DF2_SUPPORT), 1)
# EXT_LIBS += -L../../../extlib/lib/$(CROSS_NAME) -lmxml -loemtool
else
CGI_MODEL_SPEC = ../../../_config/model_spec.c
endif
CGI_MODEL_SPEC = ../../../_config/model_spec.c
SETUP_ENC = ../../../setup/setup_encryption.c
INI_IMPL = ../../../lib/ini_impl.c
LIB_NET = ../../../lib/net.c
SETUP_INI = ../../../setup/setup_ini.c
IPC_STATUS = ../../../ipc/ipc_status.c
IPC_CLIENT = ../../../ipc/ipc_client.c
DBMANAGER = ../../../lib/dbmanager.c

CGI  = login login2 logout_proc
# CGI += 
CGI += live_fview
CGI += websetup
AJAX  = new_setup
AJAX += setup_camera_setup cam_image_setup_iq cam_image_setup_ae cam_image_setup_blc cam_image_setup_awb cam_image_setup_daynight cgi_sys_upgrade cgi_sys_upgrade_2 cgi_sys_upgrade_3 login2 cgi_sys_info cgi_sys_date cgi_sys_user cgi_sys_reboot cgi_sys_default
AJAX += cgi_net_rtsp_onvif
AJAX += cgi_net_interface cgi_net_port
AJAX += cgi_ipc_video_stream    
AJAX += cgi_fw_upgrade
AJAX += video_player
TEST = cgi_sys_upgrade_2




##################################################################

all: $(FCGI_TARGETS) $(SH_CGI) $(TTY) $(CGI) _setup $(AJAX)

fcgi: $(FCGI_TARGETS)

shcgi: $(SH_CGI)

cgi_test: info

info:
	@echo "MINVER = $(MK_SYS_VER3)"

tty: $(TTY)

$(FCGI_TARGETS) : dep $(FCGI_OBJS)
	@$(call run_and_test,$(CC) $(CFLAGS) $(INC) $(LDFLAGS) $(MACRO) -o $@.fcgi $@.c $(ENCODER_DEF) $(VERIFY_USER) $(SHA256) $(SETUP_ENC) $(CGI_MODEL_SPEC) $(INI_IMPL) $(SETUP_INI) $(IPC_CLIENT) $(EXT_LIBS) -L. -ljQuery_common -lrt -lstdc++ $(FCGI_OBJS) ../../fastcgi/lib/$(CROSS_NAME)/libfcgi.a, Building)
	$(STRIP) $@.fcgi
	chmod 755 $@.fcgi

$(SH_CGI) :
	#cp $@.sh $@
	#chmod 755 $@

$(FCGI_OBJS):	%.o: %.c
	@$(call run_and_test,$(CC) -c -o $@ $< $(CFLAGS) $(INC) $(MACRO) -D__FCGI__, Compiling)

$(TTY) : dep
	@$(call run_and_test,$(CC) $(CFLAGS) $(INC) $(LDFLAGS) $(MACRO) -o $@.cgi $@.c $(ENCODER_DEF) $(VERIFY_USER) $(SHA256) $(SETUP_ENC) $(CGI_MODEL_SPEC)  $(IPC_CLIENT) $(EXT_LIBS) -L. -ljQuery_common -lrt $(OBJS), Building)
	$(STRIP) $@.cgi
	chmod 755 $@.cgi

$(CGI) : dep
	@$(call run_and_test,$(CC) $(CFLAGS) $(INC) $(LDFLAGS) $(MACRO) -o $@.cgi $@.c cgi_encryption.c $(ENCODER_DEF) $(VERIFY_USER) $(SHA256) $(SETUP_ENC) $(INI_IMPL) $(SETUP_INI) $(IPC_STATUS) $(CGI_MODEL_SPEC) $(IPC_CLIENT) $(EXT_LIBS) $(JSON_UTIL) -L. -ljQuery_common -lrt, Building)
	$(STRIP) $@.cgi
	chmod 755 $@.cgi

_setup : dep

$(AJAX) : dep dep_ajax
	@$(call run_and_test,$(CC) $(CFLAGS) $(INC) $(LDFLAGS) $(MACRO) -o $@.cgi $@.c cgi_encryption.c $(ENCODER_DEF)  $(SHA256) $(IPC_CLIENT) $(EXT_LIBS) $(SETUP_ENC) $(IPC_STATUS) $(CGI_MODEL_SPEC) $(INI_IMPL) $(SETUP_INI) $(DBMANAGER) $(JSON_UTIL) -L. -ljQuery_common -lajax_common -liconv -lrt $(OBJS), Building)
	$(STRIP) $@.cgi
	chmod 755 $@.cgi

dep :
	@$(call run_and_test,$(CC) -fPIC -c $(CFLAGS) $(INC) $(MACRO) jQuery_common.c, Compiling)
	@$(call run_and_test,$(CC) -shared -Wall -o libjQuery_common.so jQuery_common.o, Linking)
	$(STRIP) libjQuery_common.so

dep_ajax :
	@$(call run_and_test,$(CC) -fPIC -c $(CFLAGS) $(INC) $(MACRO) ajax_common.c, Compiling)
	@$(call run_and_test,$(CC) -shared -Wall -o libajax_common.so ajax_common.o, Linking)
	$(STRIP) libajax_common.so

clean :
	rm -f *.o
	rm -f *.cgi
	rm -f *.so
