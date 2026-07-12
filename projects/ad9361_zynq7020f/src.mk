# Reuse the standard AD9361 application and driver selection without copying it.
RK7020F_PROJECT := $(PROJECT)
AD9361_PROJECT := $(realpath $(PROJECT)/../ad9361)

# Make these variables simple so paths from the included file are expanded while
# PROJECT points at the standard AD9361 project.
SRCS := $(SRCS)
INCS := $(INCS)
PROJECT := $(AD9361_PROJECT)
include $(AD9361_PROJECT)/src.mk
PROJECT := $(RK7020F_PROJECT)

SRCS := $(filter-out $(AD9361_PROJECT)/src/main.c,$(SRCS))
SRCS += $(PROJECT)/src/main.c
CFLAGS += -DAD9361_PROJECT_MAIN_FILE=\"$(AD9361_PROJECT)/src/main.c\"

ifeq (y,$(strip $(IIOD)))
ifeq (xilinx,$(strip $(PLATFORM)))
NETWORKING := y
LIBRARIES += lwip
CFLAGS += -DNO_OS_LWIP_NETWORKING -DIN_ADDR_T_DEFINED \
	-DCONFIG_LINKSPEED1000 \
	-include $(PROJECT)/src/xlwipconfig.h

XILINX_VITIS_ROOT := $(patsubst %/bin/xsct,%,$(realpath $(shell command -v xsct)))
XILINX_LWIP_PORT := $(lastword $(sort $(wildcard $(XILINX_VITIS_ROOT)/../data/embeddedsw/ThirdParty/sw_services/lwip*_v*/src/lwip-*/contrib/ports/xilinx)))
ifeq ($(XILINX_LWIP_PORT),)
$(error Xilinx lwIP port not found in the Vitis installation)
endif

SRCS += $(NO-OS)/network/tcp_socket.c \
	$(PROJECT)/src/lwip_socket.c \
	$(PROJECT)/src/lwip_xilinx.c \
	$(XILINX_LWIP_PORT)/netif/xadapter.c \
	$(XILINX_LWIP_PORT)/netif/xpqueue.c \
	$(XILINX_LWIP_PORT)/netif/xemacpsif.c \
	$(XILINX_LWIP_PORT)/netif/xemacpsif_dma.c \
	$(XILINX_LWIP_PORT)/netif/xemacpsif_hw.c \
	$(XILINX_LWIP_PORT)/netif/xemacpsif_physpeed.c

INCS += $(NO-OS)/network/network_interface.h \
	$(NO-OS)/network/tcp_socket.h \
	$(NO-OS)/network/lwip_raw_socket/lwip_socket.h \
	$(PROJECT)/src/lwip_xilinx.h \
	$(PROJECT)/src/adin1110.h \
	$(PROJECT)/src/lwipopts.h \
	$(XILINX_LWIP_PORT)/include/netif/xadapter.h \
	$(PROJECT)/src/xlwipconfig.h

CFLAGS += -I$(PROJECT)/src \
	-I$(NO-OS)/libraries/lwip/configs \
	-I$(NO-OS)/libraries/lwip/arch \
	-I$(NO-OS)/network/lwip_raw_socket \
	-I$(XILINX_LWIP_PORT)/include \
	-I$(XILINX_LWIP_PORT)/netif

IGNORED_FILES += $(NO-OS)/network/linux_socket \
	$(NO-OS)/network/lwip_raw_socket \
	$(NO-OS)/network/lwip_raw_socket/netdevs/adin1110 \
	$(NO-OS)/network/w5500_network \
	$(NO-OS)/network/wifi \
	$(NO-OS)/libraries/lwip/lwip/src/netif/slipif.c
endif
endif
