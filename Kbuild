# SPDX-License-Identifier: GPL-2.0
# Convert Pimoroni Trackball in to a mouse. Software written by wallComputer.


KERN_VER = $(shell uname -r)
KDIR ?= /lib/modules/$(KERN_VER)/build
MODULE_NAME := ptMouse
KBUILD_OUTPUT = build

obj-m := $(MODULE_NAME).o

$(MODULE_NAME)-objs:= source/mod_src/$(MODULE_NAME)_main.o source/mod_src/$(MODULE_NAME)_i2cHelper.o
ccflags-y := -I$(src)/src/mod_src/