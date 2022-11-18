#!/bin/bash -x
# =============================================
# Parameters
#

function init_otbr()
{
    otbr-agent -d ${debuglevel} -I wpan0 -B mlan0 'spinel+spi:///dev/spidev1.0?gpio-reset-device=/dev/gpiochip5&gpio-int-device=/dev/gpiochip5&gpio-int-line=12&gpio-reset-line=13&spi-mode=0&spi-speed=1000000&spi-reset-delay=0'&
    sleep 2

    iptables -A FORWARD -i mlan0 -o wpan0 -j ACCEPT
    iptables -A FORWARD -i wpan0 -o mlan0 -j ACCEPT

    ip6tables -A FORWARD -i mlan0 -o wpan0 -j ACCEPT
    ip6tables -A FORWARD -i wpan0 -o mlan0 -j ACCEPT
}

function form_ot_network()
{
    echo -e "Forming OT network"

    pskc=(`pskc ${commissionerphrase} ${extpanid} ${networkname}`)

    ot-ctl dataset init new
    ot-ctl dataset channel ${channel}
    ot-ctl dataset panid ${panid}
    ot-ctl dataset extpanid ${extpanid}
    ot-ctl dataset networkname ${networkname}
    ot-ctl dataset networkkey ${networkkey}
    ot-ctl dataset pskc ${pskc}
    ot-ctl dataset commit active
    ot-ctl prefix add ${meshlocalprefix} paos med
    ot-ctl ifconfig up
    ot-ctl thread start
}

cwd=$PWD

while [ ! -d /proc/mwlan/adapter0 ]
do
    sleep 1
done
sleep 1

cd /usr/share/nxp_iw612/scripts
source config/nxp_otbr.ini
init_otbr
form_ot_network

sed -i '/^\tprefix/c\\tprefix '"$meshlocalprefix"' {' /etc/radvd.conf
systemctl restart radvd

cd $cwd
