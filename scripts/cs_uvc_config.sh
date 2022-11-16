#!/bin/sh

# check if user is root/running with sudo
if [ `whoami` != root ]; then
    echo Please run this script with sudo
    exit
fi

cd `dirname $0`
SCRIPT_PATH=`pwd`
cd $SCRIPT_PATH


# install UDEV rules for USB device
cp ${SCRIPT_PATH}/cs_uvc.rules /etc/udev/rules.d/cs_uvc.rules

# reload rules and trigger
udevadm control --reload-rules
udevadm trigger