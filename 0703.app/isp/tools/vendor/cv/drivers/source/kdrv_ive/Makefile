MODULE_NAME = kdrv_ive
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
KDRV_DIR = $(NVT_HDAL_DIR)/drivers/k_driver
KFLOW_DIR = $(NVT_HDAL_DIR)/drivers/k_flow
CURR_DRIVER_DIR = $(NVT_HDAL_DIR)/vendor/cv/drivers
RTOS_OUTPUT_DIR = $(CURR_DRIVER_DIR)/output
KDRV_DIR_OUTPUT = $(KDRV_DIR)/output
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
KDRV_INC_PATH = $(KDRV_DIR)/include
KDRV_SRC_INC_PATH = $(KDRV_DIR)/source/include
KFLOW_INC_PATH = $(KFLOW_DIR)/include
CURR_DRIVER_INC_PATH = $(CURR_DRIVER_DIR)/include/kdrv_ive
VG_MODULE_INC = $(NVT_KDRV_PATH)/source/include
KDRV_COMM_INC_PATH = $(KDRV_DIR)/include/comm

# INC FLAGs
EXTRA_INCLUDE += \
	-I$(VOS_INC_PATH) \
	-I$(KDRV_INC_PATH) \
	-I$(KDRV_SRC_INC_PATH) \
	-I$(KFLOW_INC_PATH) \
	-I$(CURR_DRIVER_INC_PATH) \
	-I$(VG_MODULE_INC) \
	-I$(NVT_HDAL_DIR)/include \
	-I$(KDRV_COMM_INC_PATH) \
	-I../../../../../../../code/application/source/test/emulation\
	-I../../../../../../../base/rtos-code/application/source/test/emulation \



ifeq ("${RTOS_CPU_TYPE}","cortex-a53")
EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_na51000
EXTRA_INCLUDE += -I../../../../../../driver/na51000/include
else
EXTRA_INCLUDE += -I$(KDRV_INC_PATH)/rtos_na51089
EXTRA_INCLUDE += -I../../../../../../driver/na51089/include
EXTRA_INCLUDE += -I../../../../../../../base/rtos-code/driver/na51089/include
endif

.PHONY: modules modules_install clean
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
C_CFLAGS  += -D__LINUX -Werror

EXTRA_CFLAGS += $(C_CFLAGS) $(EXTRA_INCLUDE) -Wno-date-time  -I$(src)/include -DDEBUG
KBUILD_EXTRA_SYMBOLS = $(shell find $(VOS_DRIVER_DIR) -name Module.symvers) $(shell find $(KDRV_DIR) -name Module.symvers) $(shell find $(KFLOW_DIR) -name Module.symvers)

obj-m += $(MODULE_NAME).o
OBJ := ive_lib.o ive_int.o ive_main.o ive_proc.o ive_drv.o ive_api.o kdrv_ive_config.o kdrv_ive_api.o ive_platform.o ive_ll_cmd.o ive_sys_vfy.o

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
EXTRA_INCLUDE += -I$(KDRV_DIR)/include/plat
C_CFLAGS = $(PLATFORM_CFLAGS) $(EXTRA_INCLUDE)
SRC = \
		kdrv_ive_config.c \
		kdrv_ive_api.c \
		ive_int.c \
		ive_lib.c \
		ive_platform.c \
		ive_ll_cmd.c \
		ive_ioctl.c \
		ive_sys_vfy_mt.c \

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
