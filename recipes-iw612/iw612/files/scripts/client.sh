#!/bin/bash -x
# =============================================
# Parameters
#

cwd=$PWD

while [ ! -d /proc/mwlan/adapter0 ]
do
    sleep 1
done
sleep 1
ifconfig mlan0 up
cd /usr/share/nxp_iw612/scripts
killall wpa_supplicant
wpa_supplicant -d -B -i mlan0 -c config/wpa_supplicant.conf
udhcpc -i mlan0 -b -t 3

echo 1 > /proc/sys/net/ipv6/conf/all/forwarding
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 2 > /proc/sys/net/ipv6/conf/all/accept_ra

cd $cwd