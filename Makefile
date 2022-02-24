# SPDX-License-Identifier: GPL-2.0
# Convert Pimoroni Trackball in to a mouse. Software written by wallComputer.

include Kbuild
CFLAGS_$(MODULE_NAME)_main.o := -DDEBUG

modules:
	make -C $(KDIR) M=$(PWD) modules

dtbo:
	dtc -I dts -O dtb -o i2c-ptMouse.dtbo source/dts_src/i2c-ptMouse.dts

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -rf i2c-ptMouse.dtbo
