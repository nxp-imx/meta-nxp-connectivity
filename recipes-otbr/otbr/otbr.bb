PN = "otbr"
SUMMARY = "OTBR on i.MX boards"
DESCRIPTION = "OTBR applications"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

#SRC_URI = "gitsm://github.com/project-chip/connectedhomeip.git;protocol=http;name=matter;branch=master"
#SRC_URI = "git://github.com/openthread/ot-br-posix.git;protocol=http;name=matter;branch=main"
#SRC_URI = "git:////home/haoran/yocto/repo/sources/meta-otbr/recipes-otbr/otbr/ot-br-posix/;branch=main;protocol=file"
#SRC_URI = "git://home/haoran/work/otbr/ot-br-posix/;protocol=file"


SRC_URI = "gitsm://github.com/openthread/ot-br-posix.git;branch=main"
#FILESEXTRAPATHS_prepend := "${THISDIR}:"
#SRC_URI = "file://ot-br-posix/"


#SRCREV = "d85d558c04e080168b2b63dc0b75e66ab563ac99"
SRCREV = "8e0ee639b174ab74ca9e90f4b2557f42336e9d8b"

S = "${WORKDIR}/git"
FILES_${PN} += "lib/systemd"
FILES_${PN} += "usr/share"
#FILES_${PN} += "${WORKDIR}/1.0-r0/recipe-sysroot-native"

#INSANE_SKIP_${PN} += "installed-vs-shipped"

DEPENDS += " jsoncpp avahi boost "
#DEPENDS += " jsoncpp avahi boost nodejs-native "
RDEPENDS_${PN} += " jsoncpp libavahi-client "

#do_configure_prepend() {
#  cd ${WORKDIR}/git
#  git submodule update --init --recursive
#}
#do_configure() {
#  cd ${WORKDIR}/git
#  git submodule update --init --recursive --depth 1
#   echo "start config"
#}
#
#do_compile() {
##   echo "start compile"
##    cd ${S}/
##    ./script/cmake-build
#}

#do_install() {
#}

#INSANE_SKIP_${PN} = "ldflags"

#FILES_${PN} += "${bindir}*"
#do_install_append() {
#    rm -rf ${WORKDIR}/recipe-sysroot-native
#    rm ${WORKDIR}/recipe-sysroot/usr/include/c++/10.2.0/fstream
#    rm ${WORKDIR}/recipe-sysroot/usr/include/avahi-ui/avahi-ui.h
#    rm ${WORKDIR}/recipe-sysroot/usr/include/json/reader.h
#    rm ${WORKDIR}/recipe-sysroot/usr/include/utmp.h
#    rm ${WORKDIR}/recipe-sysroot/usr/include/boost/mem_fn.hpp
#    rm ${WORKDIR}/recipe-sysroot/usr/include/zconf.h
#    rm ${WORKDIR}/recipe-sysroot/usr/include/asm-generic/bpf_perf_event.h
#    rm -rf ${WORKDIR}/recipe-sysroot/usr/include
#    rm -rf ${WORKDIR}/recipe-sysroot/
#
#}

inherit cmake
#EXTRA_OECMAKE = "-GNinja -DBUILD_TESTING=OFF -DOTBR_DBUS=ON -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOTBR_WEB=OFF -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_BACKBONE_ROUTER=ON -DOTBR_DBUS=OFF"
EXTRA_OECMAKE = "-GNinja -DOTBR_BORDER_ROUTING=ON -DOTBR_WEB=ON -DBUILD_TESTING=OFF -DOTBR_DBUS=OFF -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.1 -DOTBR_INFRA_IF_NAME=mlan0" 
