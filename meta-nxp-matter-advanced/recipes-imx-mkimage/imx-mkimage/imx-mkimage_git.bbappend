FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

PATCHTOOL = "git"
SRC_URI += "file://0001-Add-Trusty-prebuilt-binary-for-project-Matter.patch"
SRC_URI += "file://0001-Force-use-updated-mx93a1-ele-firmware.patch"
