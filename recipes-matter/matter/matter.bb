PN = "matter"
SUMMARY = "Matter IoT connectivity on i.MX boards"
DESCRIPTION = "This layer loads the main Matter applications"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI = "gitsm://github.com/project-chip/connectedhomeip.git;protocol=http;name=matter;branch=master"

SRCREV = "cb5d39e7087ac98c2d7ded4eb491ba998cdfe700"

DEPENDS += " gn-native ninja-native avahi python3-native dbus-glib-native "
RDEPENDS_${PN} += " libavahi-client "

S = "${WORKDIR}/git"

do_configure() {
    cd ${S}/examples/lighting-app/linux
	PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64 --args='treat_warnings_as_errors=false target_os="linux" target_cpu="arm64" arm_arch="armv8-a"
        import("//build_overrides/build.gni")
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                        "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                        "-O3"
                       ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

	cd ${S}/examples/all-clusters-app/linux
	PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64 --args='treat_warnings_as_errors=false target_os="linux" target_cpu="arm64" arm_arch="armv8-a"
        import("//build_overrides/build.gni")
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                        "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                        "-O3"
                       ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

	cd ${S}/examples/thermostat/linux
	PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64 --args='treat_warnings_as_errors=false target_os="linux" target_cpu="arm64" arm_arch="armv8-a"
        import("//build_overrides/build.gni")
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                        "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                        "-O3"
                       ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

}

do_compile() {
    cd ${S}/examples/lighting-app/linux
    ninja -C out/aarch64

	cd ${S}/examples/all-clusters-app/linux
    ninja -C out/aarch64

	cd ${S}/examples/thermostat/linux
    ninja -C out/aarch64
}

do_install() {
	install -d -m 755 ${D}${bindir}
	install ${S}/examples/lighting-app/linux/out/aarch64/chip-lighting-app ${D}${bindir}
	install ${S}/examples/all-clusters-app/linux/out/aarch64/chip-all-clusters-app ${D}${bindir}
	install ${S}/examples/thermostat/linux/out/aarch64/thermostat-app ${D}${bindir}
}

INSANE_SKIP_${PN} = "ldflags"
