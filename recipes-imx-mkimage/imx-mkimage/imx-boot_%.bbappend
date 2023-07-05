FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

PATCHTOOL = "git"
SRC_URI += "file://0001-MATTER-1246-Add-imx93-a1-sentinel-firmware.patch"
SRC_URI += "file://0001-Add-Trusty-prebuilt-binary-for-project-Matter.patch"
ATF_MACHINE_NAME:append = "${@bb.utils.contains('MACHINE_FEATURES', 'trusty', '-trusty', '', d)}"

DEPLOY_TRUSTY = "${@bb.utils.contains('MACHINE_FEATURES', 'trusty', 'true', 'false', d)}"
