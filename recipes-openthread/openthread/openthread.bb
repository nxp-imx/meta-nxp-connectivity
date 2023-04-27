PN = "openthread"
SUMMARY = "OT-DAEMON on i.MX boards"
DESCRIPTION = "OPENTHREAD applications"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=543b6fe90ec5901a683320a36390c65f"

S = "${WORKDIR}/git"
#FILES_${PN} += "lib/systemd"
#FILES_${PN} += "usr/share"

DEPENDS += " avahi boost "
RDEPENDS_${PN} += " libavahi-client "

TARGET_CFLAGS += " -Wno-error "
def get_rcp_bus(d):
    for arg in (d.getVar('MACHINE') or '').split():
        if arg == "imx93evk":
            return '-DOT_POSIX_CONFIG_RCP_BUS=SPI'
    for arg in (d.getVar('OT_RCP_BUS') or '').split():
        if arg == "SPI":
            return '-DOT_POSIX_CONFIG_RCP_BUS=SPI'
        if arg == "UART":
            return '-DOT_POSIX_CONFIG_RCP_BUS=UART'
    return ''

inherit cmake
EXTRA_OECMAKE += "${@get_rcp_bus(d)}"

do_install() {
	install -d -m 755 ${D}${bindir}
    install ${WORKDIR}/build/src/posix/ot-daemon ${D}${bindir}
    install ${WORKDIR}/build/src/posix/ot-ctl ${D}${bindir}/ot-client-ctl
}

# ot_src_rev_opts_patches_certification.inc:
# MATTER 1.0 and Thread Posix BorderRouter Certifications granted for RD-IW612 Hardware
# This file contains specific SHA1, build options and patches applied on Github Openthread repository

OT_SRC_REV_OPTS_PATCHES_INCLUDE="${@bb.utils.contains_any('MACHINE', "imx8mmevk-matter imx93evk-matter imx6ullevk", 'ot_src_rev_opts_patches_standard.inc', 'ot_src_rev_opts_patches_certification.inc', d)}"
include ${OT_SRC_REV_OPTS_PATCHES_INCLUDE}

