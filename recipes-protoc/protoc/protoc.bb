PN = "protoc-native"
SUMMARY = "protoc prebuilt tools"
DESCRIPTION = "protoc prebuilt binaries"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

PACKAGES = "${PN}"

SRC_URI = "https://github.com/protocolbuffers/protobuf/releases/download/v21.0/protoc-21.0-linux-x86_64.zip;unpack=yes"
SRC_URI[sha256sum] = "a2a92003da7b8c0c08aab530a3c1967d377c2777723482adb9d2eb38c87a9d5f"

S = "${WORKDIR}"

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
    cp -ar bin/protoc ${D}${bindir}/
    # This is a workaround to bypass the issue that protoc modified by build system
    chmod 444 ${D}${bindir}/protoc
}

do_package_qa[noexec] = "1"
EXCLUDE_FROM_SHLIBS = "1"

# This is a workaround to bypass the issue that protoc modified by build system
do_deploy() {
    chmod 755 ${D}${bindir}/protoc
}

do_populate_sdk:append() {
    chmod 755 ${D}${bindir}/protoc
}

addtask deploy after do_install do_populate_sysroot
addtask deploy before do_cleansstate
addtask deploy before do_clean

