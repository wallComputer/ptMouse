#!/bin/sh


if make && make dtbo ; then

	sudo sh -c "echo dtoverlay=i2c-ptMouse >> /boot/config.txt"
	sudo cp i2c-ptMouse.dtbo /boot/overlays/

	sudo touch /etc/modprobe.d/ptMouse.conf
	sudo sh -c "echo ptMouse >> /etc/modules"
	sudo sh -c "echo # Conf File for ptMouse > /etc/modprobe.d/ptMouse.conf"
	sudo sh -c "echo options ptMouse orientation=3 >> /etc/modprobe.d/ptMouse.conf" 
	sudo sh -c "echo options ptMouse work_rate_ms=20 >> /etc/modprobe.d/ptMouse.conf" 
	sudo sh -c "echo options ptMouse up_scale=500 >> /etc/modprobe.d/ptMouse.conf" 
	sudo sh -c "echo options ptMouse down_scale=500 >> /etc/modprobe.d/ptMouse.conf" 
	sudo sh -c "echo options ptMouse left_scale=1000 >> /etc/modprobe.d/ptMouse.conf" 
	sudo sh -c "echo options ptMouse right_scale=1000 >> /etc/modprobe.d/ptMouse.conf" 
	sudo sh -c "echo options ptMouse button_keycode=272 >> /etc/modprobe.d/ptMouse.conf" 
	
    sudo cp ptMouse.ko /lib/modules/$(uname -r)/kernel/drivers/i2c/ptMouse.ko

        sudo depmod
#	sudo insmod /lib/modules/$(uname -r)/kernel/drivers/i2c/ptMouse.ko
	echo "Files are installed. Driver will work from next reboot.\n"
else
        echo "MAKE DID NOT SUCCEED!!!\n"
fi
