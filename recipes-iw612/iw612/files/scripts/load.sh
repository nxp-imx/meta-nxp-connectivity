#!/bin/bash -x
# =============================================
# Parameters
#

cwd=$PWD

cd /usr/share/nxp_iw612/bin_sdw61x
insmod mlan.ko
insmod sdw61x.ko mod_para=config/wifi_mod_para.conf
sleep 1
./fw_loader_imx_lnx /dev/ttymxc2 115200 0 /lib/firmware/nxp/uartspi_n61x_v1.bin 3000000
sleep 1

cd $cwd
