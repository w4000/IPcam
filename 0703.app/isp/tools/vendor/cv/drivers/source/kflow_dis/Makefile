MODULE_NAME = kflow_dis
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
CURR_DRIVER_DIR = $(KDRV_DIR)
RTOS_OUTPUT_DIR = $(CURR_DRIVER_DIR)/output
KDRV_DIR_OUTPUT = $(KDRV_DIR)/output
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
NVT_CV_KDRV_PATH = $(NVT_HDAL_DIR)/vendor/cv/drivers
ECOS_OUTPUT_DIR = $(NVT_CV_KDRV_PATH)/output
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
VG_MODULE_INC = $(KDRV_DIR)/source/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(NVT_CV_KDRV_PATH)/include
# INC FLAGs
EXTRA_INCLUDE += -I$(VOS_INC_PATH) -I$(KDRV_INC_PATH) -I$(VG_MODULE_INC) -I$(KFLOW_INC_PATH) -I$(CURR_DRIVER_INC_PATH) -I$(LIBRARY_DIR)/include
EXTRA_INCLUDE += -I$(CURR_DRIVER_INC_PATH)/kdrv_dis \
				 -I$(CURR_DRIVER_INC_PATH)/kflow_dis \
				 -I$(NVT_CV_KDRV_PATH)/source/kflow_dis/include \
				 -I$(NVT_CV_KDRV_PATH)/source/kflow_dis/eth/include \
				 
ifeq ("${RTOS_CPU_TYPE}","cortex-a53")
EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_na51000
EXTRA_INCLUDE += -I../../../../../../../driver/na51000/include
else
EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_na51089
EXTRA_INCLUDE += -I../../../../../../../driver/na51089/include
endif
.PHONY: modules modules_install clean
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
C_CFLAGS  += -D__LINUX -Werror

EXTRA_CFLAGS += $(C_CFLAGS) $(EXTRA_INCLUDE) -Wno-date-time  -I$(src)/include -DDEBUG

obj-m += $(MODULE_NAME).o
OBJ := dis_alg_hw.o dis_alg_eth.o dis_alg_fw.o dis_alg.o dis_alg_api.o \
       dis_alg_ioctl.o dis_alg_proc.o dis_alg_flow_api.o dis_alg_flow.o dis_alg_task.o \
	   dis_alg_platform.o eth/kflow_eth_api.o eth/eth_lib.o \

ifeq ($(CONFIG_FUNCTION_TRACER),y)
$(MODULE_NAME)-objs := $(OBJ)
else
$(MODULE_NAME)-objs := $(OBJ:.o=_no_tracer.o)
EXTRA_CFLAGS += $(shell for n in $(OBJ:.o=); do if [ -f $(src)/$$n.c ]; then ln -sf $$(basename $${n}.c) $(src)/$${n}_no_tracer.c; else rm -f $(src)/$${n}_no_tracer.c; fi done)
endif

ifeq ($(KERNELRELEASE),)
PWD := $(shell pwd)
KERVER ?= $(NVT_LINUX_VER)
KDIR ?= $(KERNELDIR)
MDIR ?= $(KERNELDIR)/_install_modules/lib/modules/$(KERVER)/hdal
MODPATH := $(shell echo $(PWD) | awk -F'source/' '{print $$NF}';)
MODNAME := $(shell echo $(obj-m:.o=.ko))
# variables for clean object
RM_TRACER_OBJ = $(foreach n, $(OBJ:.o=), $(if $(wildcard $(n).c),$(if $(wildcard $(n).o),$(n).o,),))
RM_NO_TRACER_OBJ = $(foreach n, $(OBJ:.o=), $(if $(wildcard $(n).c),$(if $(wildcard $(n)_no_tracer.o),$(n)_no_tracer.o,),))
export KBUILD_EXTRA_SYMBOLS += $(shell find $(VOS_DRIVER_DIR) -name Module.symvers) $(shell find $(KDRV_DIR) -name Module.symvers) $(shell find $(KFLOW_DIR) -name Module.symvers)
export KBUILD_EXTRA_SYMBOLS += $(shell find $(NVT_CV_KDRV_PATH)/source -name Module.symvers)

modules:
	@$(MAKE) -C $(KDIR) M=$(PWD) $(NVT_KGCOV) modules

modules_install:
	@if [ -z $(NVT_MOD_INSTALL) ]; then \
		rm -f $(MDIR)/$(MODPATH)/$(MODNAME); \
		install -m644 -b -D $(MODNAME) ${MDIR}/$(MODPATH)/$(MODNAME); \
		cd $(KDIR)/_install_modules/lib/modules/$(KERVER)/; depmod -b $(KDIR)/_install_modules/ -a $(KERVER); \
	else \
		mkdir -p $(NVT_MOD_INSTALL)/lib/modules/$(KERVER); \
		install -m644 -b -D $(MODNAME) $(NVT_MOD_INSTALL)/lib/modules/$(KERVER)/hdal/$(MODPATH)/$(MODNAME); \
	fi

clean:
	@rm -rf $(MODULE_NAME).* .$(MODULE_NAME).* $(OBJ:.o=_no_tracer.c) Module.symvers modules.order .tmp_versions
	@rm -f `find . -type f -name ".*.cmd" -o -name ".*.d"`
	@rm -f $(RM_TRACER_OBJ) $(RM_NO_TRACER_OBJ)
codesize:
	@echo $(MODULE_NAME)
	@$(OBJDUMP) -t $(MODULE_NAME).o > $(MODULE_NAME).sym && \
	$(BUILD_DIR)/nvt-tools/nvt-ld-op -j $(MODULE_NAME).sym
endif
###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
OUTPUT_NAME = lib$(MODULE_NAME).a
C_CFLAGS = $(PLATFORM_CFLAGS) $(EXTRA_INCLUDE)
SRC = \
	dis_alg_hw.c \
	dis_alg_fw.c \
	dis_alg_eth.c \
	dis_alg_flow.c \
	dis_alg.c \
	dis_alg_api.c \
	dis_alg_flow_api.c \
	dis_alg_task.c \
	dis_alg_ioctl_rtos.c \
	eth/kflow_eth_api.c \
	eth/eth_lib.c \

copy = if [ ! -z "$(1)" -a "$(1)" != " " ]; then cp -avf $(1) $(2); fi
OBJ = $(SRC:.c=.o)
ifeq ("$(wildcard *.c */*.c */*/*.c)","")
modules:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"

clean:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
else
modules : $(OUTPUT_NAME)

%.o:%.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) -c $< -o $@
$(OUTPUT_NAME): $(OBJ)
	@echo Creating library $* ...
	@$(AR) rcsD $(OUTPUT_NAME) $(OBJ)
	@$(BUILD_DIR)/nvt-tools/nvt-ld-op --arc-sha1 $@
clean:
	@rm -f $(OBJ) *.a *.so*
endif
modules_install:
	@mkdir -p $(RTOS_OUTPUT_DIR)
	@cp -avf *.a $(RTOS_OUTPUT_DIR)
	@cp -avf *.a $(KDRV_DIR_OUTPUT)
    
endif

