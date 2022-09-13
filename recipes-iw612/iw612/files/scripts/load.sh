#!/bin/bash -x
# =============================================
# Parameters
#

cwd=$PWD

cd /usr/share/nxp_iw612/bin_sdw61x
./fw_loader_imx_lnx /dev/ttymxc2 115200 0 /lib/firmware/nxp/uartspi_n61x_v1.bin 3000000
insmod mlan.ko
insmod sdw61x.ko mod_para=/usr/share/nxp_iw612/bin_sdw61x/config/wifi_mod_para.conf

cd $cwd
