#!/bin/bash -x
# =============================================
# Parameters
#

cwd=$PWD

cd /usr/share/nxp_iw612/bin_sdw61x
insmod mlan.ko
insmod sdw61x.ko mod_para=config/wifi_mod_para.conf

cd $cwd
