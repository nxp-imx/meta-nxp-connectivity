#!/bin/bash -x
# =============================================
# Parameters
#

type=help

if [ $# -gt 0 ]; then
	source $1
fi

case "$type" in
onnetwork)
	chip-tool pairing $type $nodeid $pincode
	;;
ble-thread)
	otbr_dataset=(`ot-ctl dataset active -x`)
	chip-tool pairing $type $nodeid hex:${otbr_dataset//[$'\r\n']} $pincode $discriminator
	;;
help)
	echo "Usage:"
	echo "./pairing.sh pair_mw320.conf/pair_k32w.conf"
	echo "pair_mw320.conf: configuration file for pairing MW320"
	echo "pair_k32w.conf: configuration file for pairing K32W"
	;;
esac
