PN = "zap-native"
SUMMARY = "ZAP prebuilt tools"
DESCRIPTION = "ZAP prebuilt binaries"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

PACKAGES = "${PN}"

SRC_URI = "https://github.com/project-chip/zap/releases/download/v2023.04.27-nightly/zap-linux.zip;unpack=yes"
SRC_URI[sha256sum] = "0b49a515f0f000cdd0beb7c78a8245ce97070097ac1896c1e2d6e1c533d9d213"

S = "${WORKDIR}"

#INSANE_SKIP:${PN} = " already-stripped arch file-rdeps "
BBCLASSEXTEND = "native"
INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_SYSROOT_STRIP = "1"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_FILE_RDEPS = "1"
INHIBIT_PACKAGE_DEBUG_SPLIT_CHECK = "1"
INHIBIT_PACKAGE_DEPMODE_CHECK = "1"
INHIBIT_PACKAGE_RELOCATE = "1"
INHIBIT_PACKAGE_UNPACK = "1"

INSANE_SKIP:${PN} += "dev-so"
inherit native

do_install() {
    install -d -m 0755 ${D}${bindir}/
    cp -ar zap* ${D}${bindir}/
    # This is a workaround to bypass the issue that zap-cli modified by build system
    chmod 444 ${D}${bindir}/zap-cli
}

do_package_qa[noexec] = "1"
EXCLUDE_FROM_SHLIBS = "1"

# This is a workaround to bypass the issue that zap-cli modified by build system
do_deploy() {
    chmod 755 ${D}${bindir}/zap-cli
}

do_populate_sdk:append() {
    chmod 755 ${D}${bindir}/zap-cli
}

addtask deploy after do_install do_populate_sysroot
addtask deploy before do_cleansstate
addtask deploy before do_clean

