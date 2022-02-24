#!/bin/sh

if sudo rmmod ptMouse ; then

        sudo sed -i '/ptMouse/d' /boot/config.txt
        sudo rm -rf /boot/overlays/i2c-ptMouse.dtbo

        sudo sed -i '/ptMouse/d' /etc/modules
        sudo rm -rf /lib/modules/$(uname -r)/kernel/drivers/i2c/ptMouse.ko
        sudo rm -rf /etc/modprobe.d/ptMouse.conf

        make clean
else
        echo "ptMouse Driver is not being used.\nIf it was removed manually, remove contents from  files and locations manually.\nRefer to the contents of remover.sh to find what to remove from where.\n\n"
fi
