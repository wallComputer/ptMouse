#!/bin/sh


if make && make dtbo ; then

	sudo sh -c "echo dtoverlay=i2c-ptMouse >> /boot/config.txt"
	sudo cp i2c-ptMouse.dtbo /boot/overlays/

	sudo cp source/modconf_src/ptMouse.conf /etc/modprobe.d/ptMouse.conf
    sudo cp ptMouse.ko /lib/modules/$(uname -r)/kernel/drivers/i2c/ptMouse.ko

        sudo depmod
#	sudo insmod /lib/modules/$(uname -r)/kernel/drivers/i2c/ptMouse.ko
	echo "Files are installed. Driver will work from next reboot.\n"
else
        echo "MAKE DID NOT SUCCEED!!!\n"
fi
