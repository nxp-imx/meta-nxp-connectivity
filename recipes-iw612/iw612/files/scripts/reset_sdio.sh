#!/bin/bash -x
# =============================================
# Parameters
#

echo -n 30b50000.mmc > /sys/bus/platform/drivers/sdhci-esdhc-imx/unbind
sleep 1
echo -n 30b50000.mmc > /sys/bus/platform/drivers/sdhci-esdhc-imx/bind

