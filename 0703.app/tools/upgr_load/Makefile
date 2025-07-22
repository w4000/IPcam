# Makefile

include ../../Makefile.param
include ../../Makefile.pretty

COMPILE_OPTS =			$(INCLUDES) -I. -O2 -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64
COMPILE_OPTS +=			-D'TARGET_BIN="$(TARGET_BIN)"'
COMPILE_OPTS +=			-D'NARCH=$(NARCH)' -Wno-unused-result

C_COMPILER =			$(CROSS_COMPILE)gcc
C_FLAGS =				$(COMPILE_OPTS) -Wall -g -I$(LINUXKERNEL_INSTALL_DIR)/include
CPLUSPLUS_COMPILER =	$(CROSS_COMPILE)g++
CPLUSPLUS_FLAGS =		$(COMPILE_OPTS) -Wall -DBSD=1
LINK =					$(CROSS_COMPILE)g++ -o
CONSOLE_LINK_OPTS =		$(LINK_OPTS)
LIBRARY_LINK =			$(CROSS_COMPILE)ar cr
LIBRARY_LINK_OPTS =		$(LINK_OPTS)
STRIP = 				$(CROSS_COMPILE)strip

C_FLAGS += -I../../_sdk//$(HIARCH)/mpp/include/
C_FLAGS += -I$(MPP_PATH)/include/

CROSS_NAME = $(CROSS_COMPILE:gnueabihf-=gnueabihf)

C_FLAGS += -I../../extlib/include -I../../

ifneq ($(HKARCH), )
C_FLAGS += -D'HKARCH'
endif

SRC_FILES = sysre.c watchdog.c 

all :
	@$(call run_and_test,$(C_COMPILER) $(C_FLAGS) $(SRC_FILES) -o upgr_load -lpthread -lrt, Building)
	$(STRIP) upgr_load

clean:
	@echo Removing generated files..
	$(VERBOSE) -$(RM) -rf upgr_load
