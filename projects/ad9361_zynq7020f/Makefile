IIOD ?= y

# Board defaults. They may be overridden on the make command line.
CONFIG_NO_OS_IP ?= 192.168.0.224
CONFIG_NO_OS_NETMASK ?= 255.255.255.0
CONFIG_NO_OS_GATEWAY ?= 192.168.0.1

# Reuse the hardware definition currently maintained with the AD9361 design.
HARDWARE ?= ../ad9361/system_top.xsa

include ../../tools/scripts/generic_variables.mk
include src.mk
include ../../tools/scripts/generic.mk
